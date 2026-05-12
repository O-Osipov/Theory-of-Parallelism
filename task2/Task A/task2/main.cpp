#include <iostream>
#include <cmath>
#include <omp.h>
#include <chrono>

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

    omp_set_num_threads(potoc); // количество потоков k

    double a = -4.0; // ниж граница
    double b = 4.0; // верх граница

    double (*funk)(double);
    funk = &f;

    double res =0; // результат
    double time;


    for(int i=0; i<50; i++){
        auto start = std::chrono::high_resolution_clock::now();

        res = integrate_omp(funk, a, b, steps);

        auto end = std::chrono::high_resolution_clock::now();
        time += std::chrono::duration<double>(end - start).count();
    }
    
    double T = time/50;

    double T1 = 1.5051;
    double S = T1/T;

    std::cout << "T1: " << T1 << "\n";
    std::cout << "T: " << T << "\n";
    std::cout << "S: " << S << "\n";

    return 0;
}