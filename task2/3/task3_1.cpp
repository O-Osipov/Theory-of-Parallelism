#include <iostream>
#include <vector>
#include <cmath>
#include <omp.h>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <functional>

// Тип функции-решателя СЛАУ
using Slae = std::function<void(const std::vector<double>&, std::vector<double>&, 
                                const std::vector<double>&, int, double, double)>;

// =========================================================
// Вариант 2: Одна параллельная секция на весь алгоритм
// =========================================================
void slae_single_block(const std::vector<double>& a, std::vector<double>& x,
                       const std::vector<double>& b, int n, double tau, double eps) {
    std::vector<double> x_new(n);
    double b_norm = 0.0;
    double norm_sq = 0.0;
    bool done = false;

    #pragma omp parallel
    {
        #pragma omp for reduction(+:b_norm)
        for (int i = 0; i < n; ++i) {
            b_norm += b[i] * b[i];
        }

        #pragma omp single
        {
            b_norm = std::sqrt(b_norm);
        }
        #pragma omp barrier

        while (!done) {
            #pragma omp single
            {
                norm_sq = 0.0;
            }

            #pragma omp for reduction(+:norm_sq)
            for (int i = 0; i < n; ++i) {
                const double* row = &a[i * n];
                double Ax_i = 0.0;
                for (int j = 0; j < n; ++j) {
                    Ax_i += row[j] * x[j];
                }

                double ri = Ax_i - b[i];
                x_new[i] = x[i] - tau * ri;
                norm_sq += ri * ri;
            }

            #pragma omp for
            for (int i = 0; i < n; ++i) {
                x[i] = x_new[i];
            }

            #pragma omp single
            {
                double norm = std::sqrt(norm_sq);
                done = (norm / b_norm < eps);
            }
            #pragma omp barrier
        }
    }
}

// =========================================================
// Вариант 1: Отдельная параллельная секция для каждого цикла
// =========================================================
void slae_multiple_blocks(const std::vector<double>& a, std::vector<double>& x,
                          const std::vector<double>& b, int n, double tau, double eps) {
    std::vector<double> x_new(n);
    double b_norm = 0.0;
    bool done = false;

    #pragma omp parallel for reduction(+:b_norm)
    for (int i = 0; i < n; ++i) {
        b_norm += b[i] * b[i];
    }
    b_norm = std::sqrt(b_norm);

    while (!done) {
        double norm_sq = 0.0;

        #pragma omp parallel for reduction(+:norm_sq)
        for (int i = 0; i < n; ++i) {
            const double* row = &a[i * n];
            double Ax_i = 0.0;
            for (int j = 0; j < n; ++j) {
                Ax_i += row[j] * x[j];
            }

            double ri = Ax_i - b[i];
            x_new[i] = x[i] - tau * ri;
            norm_sq += ri * ri;
        }

        #pragma omp parallel for
        for (int i = 0; i < n; ++i) {
            x[i] = x_new[i];
        }

        double norm = std::sqrt(norm_sq);
        done = (norm / b_norm < eps);
    }
}

// =========================================================
// Универсальная функция запуска с замером времени
// =========================================================
double run_parallel(int n, Slae slae) {
    std::vector<double> a(n * n);
    std::vector<double> x(n);
    std::vector<double> b(n);
    double tau = 1.0e-5;      // оставляем как есть
    double eps = 3.0e-4;      // было 1.0e-5 → увеличили в 30 раз

    // Параллельная инициализация данных (first-touch policy для NUMA)
    #pragma omp parallel for
    for (int i = 0; i < n; ++i) {
        double* row = &a[i * n];
        for (int j = 0; j < n; ++j) {
            row[j] = 1.0;
        }
        row[i] = 2.0;              // Главная диагональ = 2.0
        b[i] = n + 1.0;            // Вектор b
        x[i] = 0.0;                // Начальное приближение
    }
    
    // Замер времени решения (только вычисления, без инициализации)
    const auto start{std::chrono::steady_clock::now()};
    slae(a, x, b, n, tau, eps);
    const auto end{std::chrono::steady_clock::now()};
    
    const std::chrono::duration<double> elapsed_seconds = end - start;
    return elapsed_seconds.count();
}

int main() {
    // === Параметры задачи ===
    // N=20000 гарантирует >= 30 сек на 1 потоке на современных CPU
    // Если время меньше на вашем железе, увеличьте до 22000-25000
    const int N = 10000;

    // === Настройки эксперимента ===
    std::vector<int> thread_counts = {1, 2, 4, 7, 8, 16, 20, 40};
    const int runs_per_thread = 10;

    // Отключаем динамическое изменение числа потоков ОС
    omp_set_dynamic(0);

    // Открываем файлы для записи результатов
    std::ofstream f_single("slae_single_blocks.csv");
    std::ofstream f_multiple("slae_multiple_block.csv");
    f_single << "threads, time\n";
    f_multiple << "threads, time\n";

    std::cout << "Запуск экспериментов. N=" << N << std::endl;

    for (int nt : thread_counts) {
        omp_set_num_threads(nt);

        for (int r = 0; r < runs_per_thread; ++r) {
            // Запуск Варианта 2 (один параллельный регион)
            double t1 = run_parallel(N, slae_single_block);

            // Запуск Варианта 1 (много параллельных регионов)
            double t2 = run_parallel(N, slae_multiple_blocks);

            // Запись в CSV (формат: threads, time)
            f_single << nt << ", " << std::fixed << std::setprecision(4) << t1 << "\n";
            f_multiple << nt << ", " << std::fixed << std::setprecision(4) << t2 << "\n";

            // Прогресс в консоль
            std::cout << "Threads: " << nt << " | Run: " << std::setw(2) << (r+1) 
                      << " | Single: " << std::fixed << std::setprecision(4) << t1 << "s"
                      << " | Multiple: " << t2 << "s\n";
        }
    }

    f_single.close();
    f_multiple.close();

    std::cout << "\n✅ Готово. Результаты сохранены:\n"
              << "  📄 slae_single_blocks.csv\n"
              << "  📄 slae_multiple_block.csv\n";
    return 0;
}