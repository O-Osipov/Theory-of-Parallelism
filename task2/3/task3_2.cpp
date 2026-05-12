#include <iostream>
#include <vector>
#include <cmath>
#include <omp.h>
#include <fstream>
#include <iomanip>
#include <algorithm>

// =========================================================
// ВАРИАНТ 1: static scheduler
// (Функция оставлена без изменений, как требовалось)
// =========================================================
void slae_static_k(
    const std::vector<double>& a, 
    std::vector<double>& x, 
    const std::vector<double>& b, 
    int n,
    double t, 
    double e,
    int k
) {
    std::vector<double> x_new(n);
    double b_norm = 0.0;
    double norm_sq = 0.0;
    bool done = false;

    #pragma omp parallel
    {
        #pragma omp for schedule(static, k) reduction(+:b_norm)
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

            #pragma omp for schedule(static, k) reduction(+:norm_sq)
            for (int i = 0; i < n; ++i) {
                const double* row = &a[i * n];
                double Ax_i = 0.0;
                for (int j = 0; j < n; ++j) {
                    Ax_i += row[j] * x[j];
                }

                double ri = Ax_i - b[i];
                x_new[i] = x[i] - t * ri;
                norm_sq += ri * ri;
            }

            #pragma omp for schedule(static, k)
            for (int i = 0; i < n; ++i) {
                x[i] = x_new[i];
            }

            #pragma omp single
            {
                double norm = std::sqrt(norm_sq);
                done = (norm / b_norm < e);
            }
            #pragma omp barrier
        }
    }
}

// =========================================================
// ВАРИАНТ 2: dynamic scheduler
// (Функция оставлена без изменений, как требовалось)
// =========================================================
void slae_dynamic(
    const std::vector<double>& a, 
    std::vector<double>& x, 
    const std::vector<double>& b, 
    int n,
    double t, 
    double e,
    int k
) {
    std::vector<double> x_new(n);
    double b_norm = 0.0;
    double norm_sq = 0.0;
    bool done = false;

    #pragma omp parallel
    {
        #pragma omp for schedule(dynamic, k) reduction(+:b_norm)
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

            #pragma omp for schedule(dynamic, k) reduction(+:norm_sq)
            for (int i = 0; i < n; ++i) {
                const double* row = &a[i * n];
                double Ax_i = 0.0;
                for (int j = 0; j < n; ++j) {
                    Ax_i += row[j] * x[j];
                }

                double ri = Ax_i - b[i];
                x_new[i] = x[i] - t * ri;
                norm_sq += ri * ri;
            }

            #pragma omp for schedule(dynamic, k)
            for (int i = 0; i < n; ++i) {
                x[i] = x_new[i];
            }

            #pragma omp single
            {
                double norm = std::sqrt(norm_sq);
                done = (norm / b_norm < e);
            }
            #pragma omp barrier
        }
    }
}

// =========================================================
// ВАРИАНТ 3: guided scheduler
// (Функция оставлена без изменений, как требовалось)
// =========================================================
void slae_guided(
    const std::vector<double>& a, 
    std::vector<double>& x, 
    const std::vector<double>& b, 
    int n,
    double t, 
    double e,
    int k
) {
    std::vector<double> x_new(n);
    double b_norm = 0.0;
    double norm_sq = 0.0;
    bool done = false;

    #pragma omp parallel
    {
        #pragma omp for schedule(guided, k) reduction(+:b_norm)
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

            #pragma omp for schedule(guided, k) reduction(+:norm_sq)
            for (int i = 0; i < n; ++i) {
                const double* row = &a[i * n];
                double Ax_i = 0.0;
                for (int j = 0; j < n; ++j) {
                    Ax_i += row[j] * x[j];
                }

                double ri = Ax_i - b[i];
                x_new[i] = x[i] - t * ri;
                norm_sq += ri * ri;
            }

            #pragma omp for schedule(guided, k)
            for (int i = 0; i < n; ++i) {
                x[i] = x_new[i];
            }

            #pragma omp single
            {
                double norm = std::sqrt(norm_sq);
                done = (norm / b_norm < e);
            }
            #pragma omp barrier
        }
    }
}

int main() {
    // === Параметры задачи ===
    const int N = 10000;               // ~30-40 сек на 1 потоке
    double tau = 1.0e-5;      
    double eps = 3.0e-4; 

    // === Конфигурация эксперимента ===
    std::vector<int> thread_counts = {2, 4, 7, 8, 16, 20, 40};
    std::vector<int> chunk_sizes   = {1, 10,  50, 100, 256}; // тестируемые k
    const int runs = 20;            // количество запусков на каждую конфигурацию

    omp_set_dynamic(0); // Отключаем динамическое изменение числа потоков ОС

    // Открываем файлы для записи
    std::ofstream f_static("slae_static.csv");
    std::ofstream f_dynamic("slae_dynamic.csv");
    std::ofstream f_guided("slae_guided.csv");

    f_static << "threads, chunks, time\n";
    f_dynamic << "threads, chunks, time\n";
    f_guided << "threads, chunks, time\n";

    // === Предвыделение данных (одинаковые для всех запусков) ===
    std::vector<double> A(N * N, 1.0);
    for (int i = 0; i < N; ++i) A[i * N + i] = 2.0;
    std::vector<double> b(N, N + 1.0);
    std::vector<double> x(N, 0.0);

    std::cout << "🚀 Запуск исследования scheduler & chunk size. N=" << N << "\n";
    std::cout << "Потоки: "; for(int t : thread_counts) std::cout << t << " "; 
    std::cout << "\nChunk sizes: "; for(int k : chunk_sizes) std::cout << k << " ";
    std::cout << "\nЗапусков на конфигурацию: " << runs << "\n";
    std::cout << std::string(80, '-') << "\n";

    for (int nt : thread_counts) {
        omp_set_num_threads(nt);
        for (int k : chunk_sizes) {
            for (int r = 0; r < runs; ++r) {
                // Сброс начального приближения перед каждым прогоном
                std::fill(x.begin(), x.end(), 0.0);
                
                // 1. Static
                double t_start = omp_get_wtime();
                slae_static_k(A, x, b, N, tau, eps, k);
                double t_static = omp_get_wtime() - t_start;

                // Сброс x
                std::fill(x.begin(), x.end(), 0.0);

                // 2. Dynamic
                t_start = omp_get_wtime();
                slae_dynamic(A, x, b, N, tau, eps, k);
                double t_dynamic = omp_get_wtime() - t_start;

                // Сброс x
                std::fill(x.begin(), x.end(), 0.0);

                // 3. Guided
                t_start = omp_get_wtime();
                slae_guided(A, x, b, N, tau, eps, k);
                double t_guided = omp_get_wtime() - t_start;

                // Запись в CSV
                f_static  << nt << ", " << k << ", " << std::fixed << std::setprecision(4) << t_static  << "\n";
                f_dynamic << nt << ", " << k << ", " << std::fixed << std::setprecision(4) << t_dynamic << "\n";
                f_guided  << nt << ", " << k << ", " << std::fixed << std::setprecision(4) << t_guided  << "\n";

                // Промежуточный вывод
                std::cout << "[Thr=" << std::setw(2) << nt << " | k=" << std::setw(3) << k << "] Run " 
                          << std::setw(2) << (r+1) << "/" << runs << " | S: " << std::setw(7) << std::fixed << std::setprecision(4) << t_static 
                          << " | D: " << std::setw(7) << t_dynamic 
                          << " | G: " << std::setw(7) << t_guided << " s\n";
            }
            std::cout << std::string(80, '.') << "\n";
        }
    }

    // Закрытие файлов
    f_static.close();
    f_dynamic.close();
    f_guided.close();

    std::cout << "\n✅ Исследование завершено. Результаты сохранены:\n"
              << "  📄 slae_static.csv\n"
              << "  📄 slae_dynamic.csv\n"
              << "  📄 slae_guided.csv\n";
              
    return 0;
}