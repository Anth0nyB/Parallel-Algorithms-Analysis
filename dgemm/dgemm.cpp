#include <omp.h>

#include <iostream>

#include "../perf.h"

#define LAPACK_INT long int

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif
void dgemm_(char *TRANSA, char *TRANSB, LAPACK_INT *m, LAPACK_INT *n, LAPACK_INT *k, double *ALPHA, double *A, LAPACK_INT *LDA, double *B, LAPACK_INT *LDB, double *BETA, double *C, LAPACK_INT *LDC);

#ifdef __cplusplus
}
#endif

void fillMat(double *mat, int len, int fill = 1) {
#pragma omp parallel for
    for (int i = 0; i < len; i++) {
        mat[i] = (double)(i * fill);
    }
}

int main(int argc, char **argv) {
    if (argc != 4) {
        cout << "usage: " << argv[0] << " <m> <k> <n>" << endl;
        return -1;
    }

    int n_threads = omp_get_max_threads();

    // Setup for PAPI
    int *event_sets = (int *)malloc(n_threads * sizeof(int));

    std::vector<std::string> events;
    int n_events = get_events(events);

    int ret = PAPI_library_init(PAPI_VER_CURRENT);
    if (ret != PAPI_VER_CURRENT) {
        fprintf(stderr, "Error inititalizing PAPI: %s\n", PAPI_strerror(ret));
    }

    // Setup for dgemm
    char Nchar = 'N';

    LAPACK_INT m = atoi(argv[1]);
    LAPACK_INT k = atoi(argv[2]);
    LAPACK_INT n = atoi(argv[3]);

    double alpha = 1;
    double beta = 0;

    LAPACK_INT lda = m;
    LAPACK_INT ldb = k;
    LAPACK_INT ldc = m;

    double *A = new double[m * k];
    double *B = new double[k * n];
    double *C = new double[m * n];

    fillMat(A, m * k);
    fillMat(B, k * n);

    // Run dgemm for all events
    double average_time = 0;
    for (int i = 0; i < n_events; i += N_SIMUL_EVENTS) {
        fillMat(C, m * n, 0);

        papi_profile_start(event_sets, events.at(i));

        double start, end;
        start = omp_get_wtime();

        dgemm_(&Nchar, &Nchar, &m, &n, &k, &alpha, A, &lda, B, &ldb, &beta, C, &ldc);

        end = omp_get_wtime();
        average_time += end - start;

        papi_profile_end(n_threads, event_sets, events.at(i));
    }
    cout << "Runtime: " << average_time / n_events << endl;

    PAPI_shutdown();
    delete[] A;
    delete[] B;
    delete[] C;

    return 0;
}
