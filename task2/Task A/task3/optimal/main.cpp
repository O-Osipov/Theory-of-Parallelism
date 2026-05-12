#include <iostream>
#include <vector>
#include <cmath>
#include <omp.h>
#include <chrono>

int main(int argc, char* argv[]){
    auto start1 = std::chrono::high_resolution_clock::now();

    int N = std::stoi(argv[1]);
    int u = std::stoi(argv[2]);

    omp_set_num_threads(10);

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

    std::vector<double> times(16);

    for(int it=0; it<50; it++){

        // A
        for(int i=0; i<N; i++){
            for(int j=0; j<N; j++){
                if (i==j){
                    A[i*N+j] = 2.0;
                }
                else{
                    A[i*N+j] = 1.0;
                }
            }
        }

        // x
        for(auto it=x.begin(); it!=x.end(); it++){
            *it=0;
        }
        // b
        for(auto it=b.begin(); it!=b.end(); it++){
            *it=N+1;
        }
        
        auto start = std::chrono::high_resolution_clock::now();

        iter = 0;
        while (iter < max_iter) {

            double norm_r = 0;
            double norm_b = 0;

            #pragma omp parallel for reduction(+:norm_r, norm_b) schedule(guided, u)
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

    double T = time/50;

    std::cout << "------" << "#pragma omp parallel for" << "------" << "\n"<< "\n";

     
    std::cout << "T: " << T << "\n";

    auto end1 = std::chrono::high_resolution_clock::now();
    auto time1 = std::chrono::duration<double>(end1 - start1).count();
    std::cout << "ALL Time: " << time1 << "\n";
    return 0;
}