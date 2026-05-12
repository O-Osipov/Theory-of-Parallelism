#include <iostream>
#include <vector>
#include <omp.h>
#include <iomanip>

/*
 * matrix_vector_product: Compute matrix-vector product c[m] = a[m][n] * b[n]
 */
void matrix_vector_product(const std::vector<double>& a, 
                           const std::vector<double>& b, 
                           std::vector<double>& c, 
                           int m, int n) {
    for (int i = 0; i < m; ++i) {
        c[i] = 0.0;
        for (int j = 0; j < n; ++j) {
            c[i] += a[i * n + j] * b[j];
        }
    }
}

double run_serial(int m, int n) {
    // Используем векторы: память выделяется в конструкторе, освобождается автоматически
    std::vector<double> a(m * n);
    std::vector<double> b(n);
    std::vector<double> c(m);
    
    // Инициализация данных
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            a[i * n + j] = static_cast<double>(i + j);
        }
    }
    for (int j = 0; j < n; ++j) {
        b[j] = static_cast<double>(j);
    }

    double t = omp_get_wtime();
    matrix_vector_product(a, b, c, m, n);
    t = omp_get_wtime() - t;
    
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Elapsed time (serial): " << t << " sec.\n";
    
    // Память освобождается автоматически при выходе из области видимости
    return t;
}

/* 
 * matrix_vector_product_omp: Parallel version with OpenMP 
 * Compute matrix-vector product c[m] = a[m][n] * b[n] 
 */
void matrix_vector_product_omp(const std::vector<double>& a, 
                               const std::vector<double>& b, 
                               std::vector<double>& c, 
                               int m, int n, int nt) {
    #pragma omp parallel num_threads(nt)
    {
        int nthreads = omp_get_num_threads();
        int threadid = omp_get_thread_num();
        int items_per_thread = m / nthreads;
        int lb = threadid * items_per_thread;
        int ub = (threadid == nthreads - 1) ? (m - 1) : (lb + items_per_thread - 1);
        
        for (int i = lb; i <= ub; ++i) {
            c[i] = 0.0;
            for (int j = 0; j < n; ++j) {
                c[i] += a[i * n + j] * b[j];
            }
        }   
    }
}

double run_parallel(int m, int n, int nt) {
    // Используем векторы вместо malloc
    std::vector<double> a(m * n);
    std::vector<double> b(n);
    std::vector<double> c(m);
    
    // Параллельная инициализация матрицы и вектора результата
    #pragma omp parallel num_threads(nt)
    {
        int nthreads = omp_get_num_threads();
        int threadid = omp_get_thread_num();
        int items_per_thread = m / nthreads;
        int lb = threadid * items_per_thread;
        int ub = (threadid == nthreads - 1) ? (m - 1) : (lb + items_per_thread - 1);

        for (int i = lb; i <= ub; ++i) {
            for (int j = 0; j < n; ++j) {
                a[i * n + j] = static_cast<double>(i + j);
            }
            c[i] = 0.0;
        }
    }

    // Инициализация вектора b (последовательно, т.к. небольшой объём)
    for (int j = 0; j < n; ++j) {
        b[j] = static_cast<double>(j);
    }

    double t = omp_get_wtime();
    matrix_vector_product_omp(a, b, c, m, n, nt);
    t = omp_get_wtime() - t;
    
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Elapsed time (parallel): " << t << " sec.\n";
    
    // Память освобождается автоматически
    return t;
}

int main(int argc, char **argv) {
    int m = 40000;
    int n = 40000;
    int num_threads = 40;
    const int num_runs = 20;  // Количество запусков для усреднения

    // // Запуск serial версии несколько раз и усреднение
    // std::cout << "Running serial version..." << std::endl;
    // double total_serial_time = 0.0;
    // for (int run = 0; run < num_runs; ++run) {
    //     std::cout << "  Run " << (run + 1) << ": ";
    //     double t = run_serial(m, n);
    //     total_serial_time += t;
    // }
    // double avg_serial_time = total_serial_time / num_runs;
    // std::cout << "Average serial time: " << avg_serial_time << " sec" << std::endl;
    // std::cout << std::endl;

    double avg_serial_time = 12.186539;


    // Запуск parallel версии несколько раз и усреднение
    std::cout << "Running parallel version..." << std::endl;
    double total_parallel_time = 0.0;
    for (int run = 0; run < num_runs; ++run) {
        std::cout << "  Run " << (run + 1) << ": ";
        double t = run_parallel(m, n, num_threads);
        total_parallel_time += t;
    }
    double avg_parallel_time = total_parallel_time / num_runs;
    std::cout << "Average parallel time: " << avg_parallel_time << " sec" << std::endl;
    std::cout << std::endl;
    
    // Расчет метрик
    double speedup = avg_serial_time / avg_parallel_time;
    double efficiency = speedup / num_threads;

     // Вывод результатов
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "=== Results ===" << std::endl;
    std::cout << "T_serial (avg):   " << avg_serial_time << " sec" << std::endl;
    std::cout << "T_parallel (avg): " << avg_parallel_time << " sec" << std::endl;
    std::cout << "Speedup (S_" << num_threads << "): " << speedup << std::endl;
    std::cout << "Efficiency (E_" << num_threads << "): " << efficiency << " (" 
              << (efficiency * 100.0) << "%)" << std::endl;


    return 0;
}