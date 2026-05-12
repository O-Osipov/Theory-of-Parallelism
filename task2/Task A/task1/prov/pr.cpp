#include <iostream>
#include <vector>
#include <omp.h>
#include <chrono>
#include <fstream>

int main(int argc, char* argv[]){
    int N = std::stoi(argv[1]); // размер масива
    int k = std::stoi(argv[2]); // число потоков

    std::ofstream fout("results.txt", std::ios::app); // file

    omp_set_num_threads(k); // количество потоков k

    std::vector<double> A(N*N); // матрица
    std::vector<double> x(N); // вектор
    std::vector<double> y(N); // результат
    double time = 0; // alg
    double time1 = 0; // init 

    for(int rt=0; rt<50; rt++){

        
        auto start1 = std::chrono::high_resolution_clock::now();
        // инициализирую матрицу и вектор
        #pragma omp parallel for schedule(static)
        for(int i=0; i<N; i++){
            x[i] = 2.0;
            y[i] = 0.0;
            for(int j=0; j<N; j++){
                A[i*N+j] = 1.0;
            }
        }
        auto end1 = std::chrono::high_resolution_clock::now();
        time1 += std::chrono::duration<double>(end1 - start1).count();

        auto start = std::chrono::high_resolution_clock::now();
        // перемножение матрицы на вектор
        #pragma omp parallel for schedule(static)
        for(int i=0; i<N; i++){
            for(int j=0; j<N; j++){
                y[i] += A[i*N+j] * x[j];
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        time += std::chrono::duration<double>(end - start).count();

    }
    
    double T_init = time1 / 50;

    double T = time/50;

    // ускорение
    // double T1 = 4.14603; 20000
    // double T1 = 26.7334; // 40000
    // double S = T1/T;
    // double E = S/k; // ttfectivity


    // std::cout << "T1: " << T1 << "\n";
    // std::cout << "T: " << T << "\n";
    // std::cout << "S: " << S << "\n";

    // std::cout << "E: " << E << "\n";

    // std::cout << "Time init: " << T_init << "\n";

    fout << N << " " << k << " " << T << " " << T_init << "\n";

    return 0;
}