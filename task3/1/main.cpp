#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <iomanip>
#include <cmath>
#include <string>
#include <cstring>

// Последовательная инициализация
void sequential_init(int N, std::vector<double>& A, std::vector<double>& B, std::vector<double>& C) {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            A[i * N + j] = std::sin(0.01 * (i + j));
        }
        C[i] = 0.0;
    }
    for (int j = 0; j < N; ++j) B[j] = std::cos(0.01 * j);
}

// Параллельная инициализация (first-touch)
void parallel_init(int tid, int nthreads, int N, 
                   std::vector<double>& A, std::vector<double>& B, std::vector<double>& C) {
    int rows_per_thread = N / nthreads;
    int start_row = tid * rows_per_thread;
    int end_row = (tid == nthreads - 1) ? N : start_row + rows_per_thread;

    for (int i = start_row; i < end_row; ++i) {
        for (int j = 0; j < N; ++j) {
            A[i * N + j] = std::sin(0.01 * (i + j));
        }
        C[i] = 0.0;
    }
    if (tid == 0) {
        for (int j = 0; j < N; ++j) B[j] = std::cos(0.01 * j);
    }
}

void sequential_matvec(int N, const std::vector<double>& A, 
                       const std::vector<double>& B, std::vector<double>& C) {
    for (int i = 0; i < N; ++i) {
        double sum = 0.0;
        for (int j = 0; j < N; ++j) {
            sum += A[i * N + j] * B[j];
        }
        C[i] = sum;
    }
}

void parallel_matvec(int tid, int nthreads, int N, 
                     const std::vector<double>& A, const std::vector<double>& B, 
                     std::vector<double>& C) {
    int rows_per_thread = N / nthreads;
    int start_row = tid * rows_per_thread;
    int end_row = (tid == nthreads - 1) ? N : start_row + rows_per_thread;

    for (int i = start_row; i < end_row; ++i) {
        double sum = 0.0;
        for (int j = 0; j < N; ++j) {
            sum += A[i * N + j] * B[j];
        }
        C[i] = sum;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <N> <threads> [--seq-init]\n";
        return 1;
    }
    
    int N = std::stoi(argv[1]);
    int nthreads = std::stoi(argv[2]);
    bool use_seq_init = (argc >= 4 && std::strcmp(argv[3], "--seq-init") == 0);

    std::vector<double> A(N * N), B(N), C(N);
    double time_init = 0.0, time_comp = 0.0;

    // === ИНИЦИАЛИЗАЦИЯ ===
    if (use_seq_init) {
        // Последовательная инициализация (для baseline)
        auto t_start = std::chrono::high_resolution_clock::now();
        sequential_init(N, A, B, C);
        auto t_end = std::chrono::high_resolution_clock::now();
        time_init = std::chrono::duration<double>(t_end - t_start).count();
    } else {
        // Параллельная инициализация (first-touch)
        std::vector<std::thread> threads;
        auto t_start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < nthreads; ++i) {
            threads.emplace_back(parallel_init, i, nthreads, N, 
                                 std::ref(A), std::ref(B), std::ref(C));
        }
        for (auto& t : threads) t.join();
        auto t_end = std::chrono::high_resolution_clock::now();
        time_init = std::chrono::duration<double>(t_end - t_start).count();
    }

    // === ВЫЧИСЛЕНИЯ ===
    if (use_seq_init || nthreads == 1) {
        // Последовательные вычисления
        auto t_start = std::chrono::high_resolution_clock::now();
        sequential_matvec(N, A, B, C);
        auto t_end = std::chrono::high_resolution_clock::now();
        time_comp = std::chrono::duration<double>(t_end - t_start).count();
    } else {
        // Параллельные вычисления
        std::vector<std::thread> threads;
        auto t_start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < nthreads; ++i) {
            threads.emplace_back(parallel_matvec, i, nthreads, N, 
                                 std::cref(A), std::cref(B), std::ref(C));
        }
        for (auto& t : threads) t.join();
        auto t_end = std::chrono::high_resolution_clock::now();
        time_comp = std::chrono::duration<double>(t_end - t_start).count();
    }

    double total_time = time_init + time_comp;
    
    std::cout << std::fixed << std::setprecision(4)
              << "Seq Time:   " << (use_seq_init ? total_time : -1.0) << " s\n"
              << "Init Time:  " << time_init << " s\n"
              << "Comp Time:  " << time_comp << " s\n"
              << "Par Total:  " << total_time << " s\n"
              << "Speedup:    " << 1.0 << "\n";

    return 0;
}