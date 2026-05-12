#include "server.h"
#include <cmath>
#include <random>
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>

// Клиент 1: вычисление синуса
void client_sin(Server<double>& server, int n_tasks, const std::string& filename) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(-100.0, 100.0);
    
    std::vector<std::pair<size_t, double>> tasks;  // id -> аргумент
    
    for (int i = 0; i < n_tasks; ++i) {
        double arg = dis(gen);
        size_t id = server.add_task([arg]() { return std::sin(arg); });
        tasks.emplace_back(id, arg);
    }
    
    // Сбор и сохранение результатов (без колонки error)
    std::ofstream out(filename);
    out << "# arg\tresult\texpected\n";
    out << std::setprecision(17);  // Максимальная точность для double
    
    for (const auto& [id, arg] : tasks) {
        double result = server.request_result(id);
        double expected = std::sin(arg);  // Эталон для записи в файл
        out << arg << "\t" << result << "\t" << expected << "\n";
    }
    std::cout << "[SIN] Saved " << n_tasks << " results to " << filename << "\n";
}

// Клиент 2: квадратный корень
void client_sqrt(Server<double>& server, int n_tasks, const std::string& filename) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(0.0, 10000.0);
    
    std::vector<std::pair<size_t, double>> tasks;
    
    for (int i = 0; i < n_tasks; ++i) {
        double arg = dis(gen);
        size_t id = server.add_task([arg]() { return std::sqrt(arg); });
        tasks.emplace_back(id, arg);
    }
    
    std::ofstream out(filename);
    out << "# arg\tresult\texpected\n";
    out << std::setprecision(17);
    
    for (const auto& [id, arg] : tasks) {
        double result = server.request_result(id);
        double expected = std::sqrt(arg);
        out << arg << "\t" << result << "\t" << expected << "\n";
    }
    std::cout << "[SQRT] Saved " << n_tasks << " results to " << filename << "\n";
}

// Клиент 3: возведение в степень
void client_pow(Server<double>& server, int n_tasks, const std::string& filename) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis_base(0.0, 10.0);
    std::uniform_real_distribution<double> dis_exp(0.0, 5.0);
    
    std::vector<std::tuple<size_t, double, double>> tasks;
    
    for (int i = 0; i < n_tasks; ++i) {
        double base = dis_base(gen);
        double exp = dis_exp(gen);
        size_t id = server.add_task([base, exp]() { return std::pow(base, exp); });
        tasks.emplace_back(id, base, exp);
    }
    
    std::ofstream out(filename);
    out << "# base\texp\tresult\texpected\n";
    out << std::setprecision(17);
    
    for (const auto& [id, base, exp] : tasks) {
        double result = server.request_result(id);
        double expected = std::pow(base, exp);
        out << base << "\t" << exp << "\t" << result << "\t" << expected << "\n";
    }
    std::cout << "[POW] Saved " << n_tasks << " results to " << filename << "\n";
}