#include <omp.h>

#include <iostream>

#include "../perf.h"

#define LAPACK_INT long int

using namespace std;

extern "C" {
extern int dgels_(char *TRANSA, LAPACK_INT *m, LAPACK_INT *n, LAPACK_INT *NRHS, double *A, LAPACK_INT *LDA, double *B, LAPACK_INT *LDB, double *WORK, LAPACK_INT *LWORK, LAPACK_INT *INFO);
}

void fillMat(double *mat, int len) {
#pragma omp parallel for
    for (int i = 0; i < len; i++) {
        mat[i] = (double)(i);
    }
}

int main(int argc, char **argv) {
    if (argc != 4) {
        cout << "usage: " << argv[0] << " <m> <n> <NHRS>" << endl;
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

    // Setup for dgels
    LAPACK_INT m = atoi(argv[1]);
    LAPACK_INT n = atoi(argv[2]);
    LAPACK_INT NRHS = atoi(argv[3]);
    LAPACK_INT lda = m;
    LAPACK_INT ldb = m > n ? m : n;
    LAPACK_INT info;
    LAPACK_INT lwork = -1;

    char Nchar = 'N';

    double *A = new double[lda * n];
    double *B = new double[ldb * NRHS];

    // Get optimal lwork value
    double wkopt;
    dgels_(&Nchar, &m, &n, &NRHS, A, &lda, B, &ldb, &wkopt, &lwork, &info);
    lwork = (LAPACK_INT)wkopt;

    fillMat(A, lda * n);
    fillMat(B, ldb * NRHS);
    double *work;

    // Get counter data for each event
    double average_time = 0;
    for (int i = 0; i < n_events; i += N_SIMUL_EVENTS) {
        info = 0;

        work = new double[lwork];

        papi_profile_start(event_sets, events.at(i));

        double start, end;
        start = omp_get_wtime();

        // Run dgels
        dgels_(&Nchar, &m, &n, &NRHS, A, &lda, B, &ldb, work, &lwork, &info);

        end = omp_get_wtime();
        average_time += end - start;

        papi_profile_end(n_threads, event_sets, events.at(i));

        delete[] work;
    }
    cout << "Runtime: " << average_time / n_events << endl;

    PAPI_shutdown();
    delete[] A;
    delete[] B;
    free(event_sets);

    return (int)info;
}
