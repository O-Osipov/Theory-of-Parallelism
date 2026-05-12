#include <iostream>
#include <omp.h>

int main() {
    std::cout << "Max threads: " << omp_get_max_threads() << std::endl;
    std::cout << "Num procs: " << omp_get_num_procs() << std::endl;
}