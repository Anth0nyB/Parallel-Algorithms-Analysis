#include <omp.h>

#include <iostream>
#include <vector>

#include "papi.h"

#define LAPACK_INT long int

using namespace std;

extern "C" {
extern int dgeqrf_(LAPACK_INT *m, LAPACK_INT *n, double *A, LAPACK_INT *lda, double *tau, double *work, LAPACK_INT *lwork, LAPACK_INT *info);
}

int papi_setup(int &n_events) {
    PAPI_library_init(PAPI_VER_CURRENT);

    // Holds the counters to be measured
    int event_set = PAPI_NULL;
    PAPI_create_eventset(&event_set);
    std::vector<int> events = {PAPI_TOT_CYC, PAPI_TOT_INS};
    n_events = events.size();
    PAPI_add_events(event_set, events.data(), events.size());

    return event_set;
}

void papi_cleanup(int &event_set) {
    PAPI_cleanup_eventset(event_set);
    PAPI_destroy_eventset(&event_set);
    PAPI_shutdown();
}

void fillMat(double *mat, int len) {
#pragma omp parallel for
    for (int i = 0; i < len; i++) {
        mat[i] = (double)(i);
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        cout << "usage: " << argv[0] << " <m> <n>" << endl;
        return -1;
    }

    LAPACK_INT m = atoi(argv[1]);
    LAPACK_INT n = atoi(argv[2]);
    LAPACK_INT lda = m;
    LAPACK_INT info = 0;
    LAPACK_INT lwork = -1;

    double wkopt;

    double *A = new double[lda * n];
    double *tau = new double[m > n ? m : n];
    double *work = new double[1];

    dgeqrf_(&m, &n, A, &lda, tau, &wkopt, &lwork, &info);
    lwork = (LAPACK_INT)wkopt;
    delete[] work;
    work = new double[lwork];

    fillMat(A, lda * n);

    int n_events;
    int event_set = papi_setup(n_events);
    PAPI_start(event_set);

    dgeqrf_(&m, &n, A, &lda, tau, work, &lwork, &info);

    PAPI_stop(event_set, nullptr);

    // Read and print the counter values
    long long counters[n_events];
    PAPI_read(event_set, counters);
    std::cout << counters[0] << "," << counters[1];

    papi_cleanup(event_set);

    delete[] A;
    delete[] tau;
    delete[] work;

    return (int)info;
}