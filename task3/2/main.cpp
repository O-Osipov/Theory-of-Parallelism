#include "server.h"
#include <iostream>
#include <thread>
#include <chrono>

// Объявления клиентов
void client_sin(Server<double>&, int, const std::string&);
void client_sqrt(Server<double>&, int, const std::string&);
void client_pow(Server<double>&, int, const std::string&);

int main() {
    constexpr int N = 10000;  // 5 < N < 10000
    
    std::cout << "Starting server with 3 clients, " << N << " tasks each...\n";
    
    Server<double> server;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    server.start();
    
    // Запуск клиентов в отдельных потоках
    std::thread t1(client_sin, std::ref(server), N, "results_sin.txt");
    std::thread t2(client_sqrt, std::ref(server), N, "results_sqrt.txt");
    std::thread t3(client_pow, std::ref(server), N, "results_pow.txt");
    
    // Ожидание завершения
    t1.join();
    t2.join();
    t3.join();
    
    server.stop();
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    
    std::cout << "\n=== Execution Summary ===\n";
    std::cout << "Total time: " << duration << " ms\n";
    std::cout << "Processed tasks: " << server.get_processed_count() << "\n";
    std::cout << "Results saved to: results_sin.txt, results_sqrt.txt, results_pow.txt\n";
    
    return 0;
}