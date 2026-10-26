#include <iostream>
#include "Timer.h"
#include <random>
#include <omp.h>

#define SEED 35791246

int main(int argc, char* argv[]) {
    int nthreads = argc > 1 ? atoi(argv[1]) : 1;
    int niter;

    std::cout << "Enter the number of iterations used to estimate pi: ";
    std::cin >> niter;

    omp_set_num_threads(nthreads);

    long long total = 4LL * niter;
    long long inside = 0;

    Timer timer;
    timer.start();

    #pragma omp parallel reduction(+:inside)
    {
        int tid = omp_get_thread_num();

        std::mt19937 generator(SEED + tid);
        std::uniform_real_distribution<double> distribution(0.0, 1.0);

        #pragma omp for
        for (long long i = 0; i < total; i++) {
            double x = distribution(generator);
            double y = distribution(generator);

            if (x * x + y * y <= 1.0) {
                inside++;
            }
        }
    }

    double pi = 4.0 * static_cast<double>(inside) / total;

    std::cout << "elapsed time for pi = " << timer.lap() << std::endl;
    std::cout << "estimate of pi is " << pi << std::endl;

    return 0;
}