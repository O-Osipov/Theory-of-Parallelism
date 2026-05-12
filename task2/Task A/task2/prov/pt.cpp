#include <iostream>
#include <cmath>
#include <omp.h>
#include <chrono>
#include <fstream>

double f(double x){
    return exp(-x*x);
}

    double integrate_omp(double (*funk)(double), double a, double b, int steps){

        double dx = (b-a)/steps;
        // double res = 0;
        double sum = 0;

        #pragma omp parallel
        {
            double local_sum = 0.0;

            #pragma omp for
            for (int i=0; i<steps; i++){
                local_sum += funk(a+i*dx) * dx;
            }

            #pragma omp atomic
            sum+=local_sum;
        }

        return sum;
    }


int main(int argc, char* argv[]){

    int steps = std::stoi(argv[1]); // число маленьких отрезков
    int potoc = std::stoi(argv[2]); // число потоков

    std::ofstream fout("results.txt", std::ios::app); // file

    omp_set_num_threads(potoc); // количество потоков k

    double a = -4.0; // ниж граница
    double b = 4.0; // верх граница

    double (*funk)(double);
    funk = &f;

    double res =0; // результат
    double time = 0;


    for(int i=0; i<50; i++){
        auto start = std::chrono::high_resolution_clock::now();

        res = integrate_omp(funk, a, b, steps);

        auto end = std::chrono::high_resolution_clock::now();
        time += std::chrono::duration<double>(end - start).count();
    }
    
    double T = time/50;
    int N = 40000000;

   fout << N << " " << potoc << " " << T << "\n";

    return 0;
}