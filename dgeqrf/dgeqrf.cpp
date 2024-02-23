#include <omp.h>

#include <iostream>

#include "../perf.h"

#define LAPACK_INT long int

using namespace std;

extern "C" {
extern int dgeqrf_(LAPACK_INT *m, LAPACK_INT *n, double *A, LAPACK_INT *lda, double *tau, double *work, LAPACK_INT *lwork, LAPACK_INT *info);
}

void fillMat(double *mat, int len) {
#pragma omp parallel for num_threads(48)
    for (int i = 0; i < len; i++) {
        mat[i] = (double)(i);
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

    // Setup for dgeqrf
    LAPACK_INT m = atoi(argv[1]);
    LAPACK_INT n = atoi(argv[2]);
    LAPACK_INT lda = m;
    LAPACK_INT info = 0;
    LAPACK_INT lwork = -1;

    double *A = new double[lda * n];
    double *tau = new double[m > n ? m : n];

    // Get optimal lwork value
    double wkopt;
    dgeqrf_(&m, &n, A, &lda, tau, &wkopt, &lwork, &info);
    lwork = (LAPACK_INT)wkopt;

    fillMat(A, lda * n);
    double *work;

    cout << n_threads << " " << m / 1000 << "k " << n / 1000 << "k ,";

    double avg_time = 0;

    bool repeat = false;
    for (int i = 0; i < n_events; ++i) {
        info = 0;
        work = new double[lwork];

        if (!papi_profile_start(event_sets, events.at(i), repeat)) {
            // Sometimes events fail to add on one thread, so try once more if this is the case
            papi_profile_end(n_threads, event_sets, events.at(i), false);
            fprintf(stderr, "Error setting up event: %s\nTrying once more\n", events.at(i).c_str());
            delete[] work;
            i--;
            repeat = true;
            continue;
        }

        avg_time -= omp_get_wtime();

        dgeqrf_(&m, &n, A, &lda, tau, work, &lwork, &info);

        avg_time += omp_get_wtime();

        papi_profile_end(n_threads, event_sets, events.at(i), true);

        delete[] work;

        repeat = false;
    }

    cout << avg_time / n_events << "," << endl;

    PAPI_shutdown();
    delete[] A;
    delete[] tau;

    return 0;
}