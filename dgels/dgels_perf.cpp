#include <omp.h>

#include <iostream>
#include <vector>

#include "papi.h"

#define LAPACK_INT long int

using namespace std;

extern "C" {
extern int dgels_(char *TRANSA, LAPACK_INT *m, LAPACK_INT *n, LAPACK_INT *NRHS, double *A, LAPACK_INT *LDA, double *B, LAPACK_INT *LDB, double *WORK, LAPACK_INT *LWORK, LAPACK_INT *INFO);
}

int papi_setup(int &n_events) {
    int retval = PAPI_library_init(PAPI_VER_CURRENT);
    if (retval != PAPI_VER_CURRENT) {
        fprintf(stderr, "Error inititalizing PAPI! %s\n", PAPI_strerror(retval));
    }

    // Holds the counters to be measured [wrong]
    int event_set[100];
    PAPI_create_eventset(event_set);
    // std::vector<int> events = {PAPI_TOT_CYC, PAPI_TOT_INS};
    // n_events = events.size();
    // PAPI_add_events(event_set, events.data(), events.size());

    // return event_set;

    int num_threads = omp_get_num_procs();
    for (int i = 0; i < num_threads; i++) {
        event_set[i] = PAPI_NULL;
        // printf("eventset[%d]: %d\n", i, eventset[i]);
    }

#pragma omp parallel
    {
        retval = PAPI_thread_init(pthread_self);
        if (retval != PAPI_OK) {
            fprintf(stderr, "Error thread init\n");
        }

        int thread_id = omp_get_thread_num();

        retval = PAPI_create_eventset(&event_set[thread_id]);
        if (retval != PAPI_OK) {
            fprintf(stderr, "Error creating eventset! %s\n", PAPI_strerror(retval));
        }

        retval = PAPI_add_named_event(event_set[thread_id], event_name);
        if (retval != PAPI_OK) {
            fprintf(stderr, "Error adding event: %s\n", PAPI_strerror(retval));
        }

        PAPI_reset(event_set[thread_id]);
        retval = PAPI_start(event_set[thread_id]);
        if (retval != PAPI_OK) {
            fprintf(stderr, "Error starting: %s\n", PAPI_strerror(retval));
        }
    }
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
    if (argc != 4) {
        cout << "usage: " << argv[0] << " <m> <n> <NHRS>" << endl;
        return -1;
    }

    LAPACK_INT m = atoi(argv[1]);
    LAPACK_INT n = atoi(argv[2]);
    LAPACK_INT NRHS = atoi(argv[3]);
    LAPACK_INT lda = m;
    LAPACK_INT ldb = m > n ? m : n;
    LAPACK_INT info = 0;
    LAPACK_INT lwork = -1;

    char Nchar = 'N';

    double wkopt;

    double *work = new double[1];

    double *A = new double[lda * n];
    double *B = new double[ldb * NRHS];

    dgels_(&Nchar, &m, &n, &NRHS, A, &lda, B, &ldb, &wkopt, &lwork, &info);
    lwork = (LAPACK_INT)wkopt;
    delete[] work;
    work = new double[lwork];

    fillMat(A, lda * n);
    fillMat(B, ldb * NRHS);

    int n_events;
    int event_set = papi_setup(n_events);
    PAPI_start(event_set);

    dgels_(&Nchar, &m, &n, &NRHS, A, &lda, B, &ldb, work, &lwork, &info);

    PAPI_stop(event_set, nullptr);

    // Read and print the counter values
    long long counters[n_events];
    PAPI_read(event_set, counters);
    std::cout << counters[0] << "," << counters[1];

    papi_cleanup(event_set);

    delete[] A;
    delete[] B;
    delete[] work;

    return (int)info;
}