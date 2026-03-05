#include <stdio.h>
#include <omp.h>
#include <cstdlib>

/*
* matrix_vector_product: Compute matrix-vector product c[m] = a[m][n] * b[n]
*/
void matrix_vector_product(double *a, double *b, double *c, int m, int n){
    for (int i = 0; i < m; i++) {
        c[i] = 0.0;
        for (int j = 0; j < n; j++)
            c[i] += a[i * n + j] * b[j];
    }
}

double run_serial(int m, int n){
    double *a, *b, *c;
    a = (double*)malloc(sizeof(*a) * m * n);
    b = (double*)malloc(sizeof(*b) * n);
    c = (double*)malloc(sizeof(*c) * m);
    
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++)
            a[i * n + j] = i + j;
    }
    for (int j = 0; j < n; j++)
        b[j] = j;

    double t = omp_get_wtime();
    matrix_vector_product(a, b, c, m, n);
    t = omp_get_wtime() - t;
    printf("Elapsed time (serial): %.6f sec.\n", t);
    free(a);
    free(b);
    free(c);

    return t;
}

/* matrix_vector_product_omp: Compute matrix-vector product c[m] = a[m][n] * b[n] */
void matrix_vector_product_omp(double *a, double *b, double *c, int m, int n, int nt){
    #pragma omp parallel num_threads(nt)
    {
        int nthreads = omp_get_num_threads();
        int threadid = omp_get_thread_num();
        int items_per_thread = m / nthreads;
        int lb = threadid * items_per_thread;
        int ub = (threadid == nthreads - 1) ? (m - 1) : (lb + items_per_thread - 1);
        
        for (int i = lb; i <= ub; i++) {
            c[i] = 0.0;
            for (int j = 0; j < n; j++)
                c[i] += a[i * n + j] * b[j];
        }   
    }
}

double run_parallel(int m, int n, int nt){
    double *a, *b, *c;
    a = (double*)malloc(sizeof(*a) * m * n);
    b = (double*)malloc(sizeof(*b) * n);
    c = (double*)malloc(sizeof(*c) * m);
    
    #pragma omp parallel num_threads(nt)
    {
        int nthreads = omp_get_num_threads();
        int threadid = omp_get_thread_num();
        int items_per_thread = m / nthreads;
        int lb = threadid * items_per_thread;
        int ub = (threadid == nthreads - 1) ? (m - 1) : (lb + items_per_thread - 1);

        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++)
                a[i * n + j] = i + j;
            c[i] = 0.0;
        }
    }

    for (int j = 0; j < n; j++)
        b[j] = j;

    double t = omp_get_wtime();
    matrix_vector_product_omp(a, b, c, m, n, nt);
    t = omp_get_wtime() - t;
    printf("Elapsed time (parallel): %.6f sec.\n", t);
    free(a);
    free(b);
    free(c);
    return t;
}

int main(int argc, char **argv){
    int m = 40000;
    int n = 40000;
    int num_threads = 40;
    // double t_serial = run_serial(m, n);
    double t_serial = 4.570312;

    double t_parallel = run_parallel(m, n, num_threads);
    printf("S_%d: %.6f\n", num_threads, t_serial/t_parallel);

    return 0;
}