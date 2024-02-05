#include <omp.h>

#include <iostream>

#include "../perf.h"

#define LAPACK_INT long int

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif
void dsymm_(char *SIDE, char *UPLO, LAPACK_INT *m, LAPACK_INT *n, double *ALPHA, double *A, LAPACK_INT *LDA, double *B, LAPACK_INT *LDB, double *BETA, double *C, LAPACK_INT *LDC);

#ifdef __cplusplus
}
#endif

void fillMat(double *mat, int len, int fill = 1) {
#pragma omp parallel for
    for (int i = 0; i < len; i++) {
        mat[i] = (double)(i * fill);
    }
}

void fillSym(double *mat, int m, int n) {
#pragma omp parallel for
    for (int row = 0; row < m; row++) {
        for (int col = 0; col < n; col++) {
            mat[row * n + col] = row * col;
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        cout << "usage: " << argv[0] << " <m> <n>" << endl;
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

    // Setup for dsymm
    char Lchar = 'L';
    char Uchar = 'U';

    LAPACK_INT m = atoi(argv[1]);
    LAPACK_INT n = atoi(argv[2]);

    double alpha = 1;
    double beta = 0;

    LAPACK_INT lda = m;
    LAPACK_INT ldb = m;
    LAPACK_INT ldc = m;

    double *A = new double[m * m];
    double *B = new double[m * n];
    double *C = new double[m * n];

    fillSym(A, m, m);
    fillMat(B, m * n);

    // Run dsymm for all events
    double average_time = 0;
    for (int i = 0; i < n_events; ++i) {
        fillMat(C, m * n, 0);

        papi_profile_start(event_sets, events.at(i));

        double start, end;
        start = omp_get_wtime();

        dsymm_(&Lchar, &Uchar, &m, &n, &alpha, A, &lda, B, &ldb, &beta, C, &ldc);

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
