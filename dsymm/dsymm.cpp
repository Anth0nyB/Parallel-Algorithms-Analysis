#include <omp.h>

#include <iostream>

#include "../perf.h"

#define LAPACK_INT long int

using namespace std;

extern "C" {
void dsymm_(char *SIDE, char *UPLO, LAPACK_INT *m, LAPACK_INT *n, double *ALPHA, double *A, LAPACK_INT *LDA, double *B, LAPACK_INT *LDB, double *BETA, double *C, LAPACK_INT *LDC);
}

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

    cout << n_threads << " " << m / 1000 << "k " << n / 1000 << "k ,";

    double avg_time = 0;

    // Run dsymm for all events
    bool repeat = false;
    for (int i = 0; i < n_events; ++i) {
        fillMat(C, m * n, 0);

        if (!papi_profile_start(event_sets, events.at(i), repeat)) {
            // Sometimes events fail to add on one thread, so try once more if this is the case
            papi_profile_end(n_threads, event_sets, events.at(i), false);
            fprintf(stderr, "Error setting up event: %s\nTrying once more\n", events.at(i).c_str());
            i--;
            repeat = true;
            continue;
        }

        avg_time -= omp_get_wtime();

        dsymm_(&Lchar, &Uchar, &m, &n, &alpha, A, &lda, B, &ldb, &beta, C, &ldc);

        avg_time += omp_get_wtime();

        papi_profile_end(n_threads, event_sets, events.at(i), true);

        repeat = false;
    }

    cout << avg_time / n_events << "," << endl;

    PAPI_shutdown();
    delete[] A;
    delete[] B;
    delete[] C;

    return 0;
}
