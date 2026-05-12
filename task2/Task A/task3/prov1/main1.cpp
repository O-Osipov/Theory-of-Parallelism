#include <iostream>
#include <vector>
#include <cmath>
#include <omp.h>
#include <chrono>
#include <fstream>

int main(int argc, char* argv[]){
    auto start1 = std::chrono::high_resolution_clock::now();

    std::ofstream fout("results.txt", std::ios::app); // file

    int N = std::stoi(argv[1]);
    int k = std::stoi(argv[2]);

    double e = 0.000001;
    double t = 0.01;

    std::vector<double> A(N*N);
    std::vector<double> x(N);
    std::vector<double> b(N);

    // для промеж вычеслений
    std::vector<double> x_new(N);


    int iter = 0;
    int max_iter = 1000;

    double time =0;
    double time_init =0;

    std::vector<double> times(16);

    omp_set_num_threads(k);

    for(int it=0; it<50; it++){

        auto start2 = std::chrono::high_resolution_clock::now();
        // A
        #pragma omp parallel for
        for(int i=0; i<N; i++){
            x[i] = 0;
            b[i] = N+1;
            for(int j=0; j<N; j++){
                if (i==j){
                    A[i*N+j] = 2.0;
                }
                else{
                    A[i*N+j] = 1.0;
                }
            }
        }
        auto end2 = std::chrono::high_resolution_clock::now();
        time_init += std::chrono::duration<double>(end2 - start2).count();


        auto start = std::chrono::high_resolution_clock::now();

        iter = 0;
        while (iter < max_iter) {

            double norm_r = 0;
            double norm_b = 0;

            #pragma omp parallel for reduction(+:norm_r, norm_b)
            for (int i = 0; i < N; i++) {

                double Ax_i = 0;

                for (int j = 0; j < N; j++)
                    Ax_i += A[i*N+j] * x[j];

                double r_i = Ax_i - b[i];

                norm_r += r_i * r_i;
                norm_b += b[i] * b[i];

                x_new[i] = x[i] - t * r_i;
            }

            if (sqrt(norm_r) / sqrt(norm_b) < e){
                break;
            }

            x.swap(x_new); // меняем указатели местами

            iter++;
        }

        auto end = std::chrono::high_resolution_clock::now();
        time += std::chrono::duration<double>(end - start).count();
    }

    double T = time / 50;
    double T_init = time_init / 50;

    auto end1 = std::chrono::high_resolution_clock::now();
    auto time1 = std::chrono::duration<double>(end1 - start1).count();

    // Теперь записываем в файл вместе с остальным
    fout << N << " " << k << " " << T << " " << T_init << "\n";
    std::cout << time1;
    
    return 0;
}