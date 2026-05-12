#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <omp.h>

double func(double x){
    return std::exp(-x * x);
}

double integrate(double (*func)(double), double a, double b, int n)
{
    double h = (b - a) / n;
    double sum = 0.0;
    
    for (int i = 0; i < n; i++)
        sum += func(a + h * (i + 0.5));
    
    sum *= h;
    return sum;
}

double integrate_omp(double (*func)(double), double a, double b, int n, int num_threads){
    double h = (b - a) / n;
    double sum = 0.0;
    
    #pragma omp parallel num_threads(num_threads)
    {
        int nthreads = omp_get_num_threads();
        int threadid = omp_get_thread_num();
        int items_per_thread = n / nthreads;
        int lb = threadid * items_per_thread;
        int ub = (threadid == nthreads - 1) ? (n - 1) : (lb + items_per_thread - 1);
        double sumloc = 0.0;

        for (int i = lb; i <= ub; i++)
            sumloc += func(a + h * (i + 0.5));

        #pragma omp atomic
        sum += sumloc;
    }
    sum *= h;
    return sum;
}

const double PI = 3.14159265358979323846;
const double a = -4.0;
const double b = 4.0;
const int nsteps = 40000000;

double run_serial(){
    double t = omp_get_wtime();
    double res = integrate(func, a, b, nsteps);
    t = omp_get_wtime() - t;
    std::cout << std::fixed << std::setprecision(12);
    std::cout << "Result (serial): " << res << "; error " << std::fabs(res - std::sqrt(PI)) << "\n";
    return t;
}

double run_parallel(int num_threads){
    double t = omp_get_wtime();
    double res = integrate_omp(func, a, b, nsteps, num_threads);
    t = omp_get_wtime() - t;
    std::cout << std::fixed << std::setprecision(12);
    std::cout << "Result (parallel): " << res << "; error " << std::fabs(res - std::sqrt(PI)) << "\n";
    return t;
}

int main(int argc, char **argv){
    int num_threads = 40;
    const int num_runs = 50;  // Количество запусков для усреднения

    std::cout << std::fixed << std::setprecision(12);
    std::cout << "Integration f(x) on [" << a << ", " << b << "], nsteps = " << nsteps << "\n";
    

    // Запуск serial версии несколько раз и усреднение
    // std::cout << "Running serial version..." << std::endl;
    // double total_serial_time = 0.0;
    // for (int run = 0; run < num_runs; ++run) {
    //     std::cout << "  Run " << (run + 1) << ": ";
    //     double tserial = run_serial();
    //     total_serial_time += tserial;
    // }
    // double avg_serial_time = total_serial_time / num_runs;
    // std::cout << "Average serial time: " << avg_serial_time << " sec" << std::endl;
    // std::cout << std::endl;

    double avg_serial_time = 0.475517;

    // Запуск parallel версии несколько раз и усреднение
    std::cout << "Running parallel version..." << std::endl;
    double total_parallel_time = 0.0;
    for (int run = 0; run < num_runs; ++run) {
        std::cout << "  Run " << (run + 1) << ": ";
        double tparallel = run_parallel(num_threads);
        total_parallel_time += tparallel;
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