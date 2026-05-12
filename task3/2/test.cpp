#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <string>

constexpr double EPSILON = 1e-9;

inline bool approx_equal(double a, double b) {
    return std::abs(a - b) < EPSILON;
}

// Проверяет только соответствие result == expected из файла (без пересчёта!)
bool test_file(const std::string& filename, bool has_two_args = false) {
    std::ifstream in(filename);
    if (!in.is_open()) {
        std::cerr << "[FAILED] Cannot open " << filename << "\n";
        return false;
    }

    std::string line;
    int total = 0, errors = 0;

    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        double arg1 = 0.0, arg2 = 0.0, result = 0.0, expected = 0.0;

        // Парсим строку в зависимости от формата
        if (has_two_args) {
            // Формат: base  exp  result  expected
            iss >> arg1 >> arg2 >> result >> expected;
        } else {
            // Формат: arg  result  expected
            iss >> arg1 >> result >> expected;
        }

        // Сравниваем результат сервера с эталоном из файла
        if (!approx_equal(result, expected)) {
            if (has_two_args) {
                std::cerr << "[FAILED] " << filename << ": pow(" << arg1 << "," << arg2 
                          << ") | result=" << result << " != expected=" << expected << "\n";
            } else {
                std::cerr << "[FAILED] " << filename << ": f(" << arg1 
                          << ") | result=" << result << " != expected=" << expected << "\n";
            }
            errors++;
        }
        total++;
    }

    std::cout << (errors == 0 ? "[OK]" : "[FAILED]") 
              << " " << filename << ": " << (total - errors) << "/" << total << " passed\n";
    return errors == 0;
}

int main() {
    std::cout << "\n=== Running Tests (verifying pre-calculated files) ===\n";

    bool all_passed = true;
    // false -> 1 аргумент (sin, sqrt), true -> 2 аргумента (pow)
    all_passed &= test_file("results_sin.txt", false);
    all_passed &= test_file("results_sqrt.txt", false);
    all_passed &= test_file("results_pow.txt", true);

    std::cout << "\n" << (all_passed ? "All tests PASSED!" : "Some tests FAILED!") << "\n";
    return all_passed ? 0 : 1;
}