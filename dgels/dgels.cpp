#include <omp.h>

#include <fstream>
#include <iostream>
#include <vector>

#include "papi.h"

#define LAPACK_INT long int
#define native_events_path "../events/native_events.txt"
#define N_SIMUL_EVENTS 1

using namespace std;

extern "C" {
extern int dgels_(char *TRANSA, LAPACK_INT *m, LAPACK_INT *n, LAPACK_INT *NRHS, double *A, LAPACK_INT *LDA, double *B, LAPACK_INT *LDB, double *WORK, LAPACK_INT *LWORK, LAPACK_INT *INFO);
}

vector<string> events;

int n_events;

int n_threads = omp_get_max_threads();

int *event_sets = (int *)malloc(n_threads * sizeof(int));

void papi_profile_start(string event_name) {
    // Each thread adds event to count
#pragma omp parallel
    {
        int thread_id = omp_get_thread_num();

        event_sets[thread_id] = PAPI_NULL;

        int ret = PAPI_thread_init(pthread_self);
        if (ret != PAPI_OK) {
            fprintf(stderr, "Error initializing thread %d\n", thread_id);
        }

        ret = PAPI_create_eventset(&event_sets[thread_id]);
        if (ret != PAPI_OK) {
            fprintf(stderr, "Error creating eventset: %s\n", PAPI_strerror(ret));
        }

        ret = PAPI_add_named_event(event_sets[thread_id], event_name.c_str());
        if (ret != PAPI_OK) {
            fprintf(stderr, "Error adding event \"%s\": %s\n", event_name.c_str(), PAPI_strerror(ret));
        }

        PAPI_reset(event_sets[thread_id]);
        ret = PAPI_start(event_sets[thread_id]);
        if (ret != PAPI_OK) {
            fprintf(stderr, "Error starting: %s\n", PAPI_strerror(ret));
        }
    }
}

void papi_profile_end(int first_event_id) {
    // Counts of the events being measured
    long long totals[N_SIMUL_EVENTS];
    for (int i = 0; i < N_SIMUL_EVENTS; i++) {
        totals[i] = 0;
    }

// Each thread stops counting event and cleans up
#pragma omp parallel
    {
        long long thread_counts[N_SIMUL_EVENTS];
        for (int i = 0; i < N_SIMUL_EVENTS; i++) {
            thread_counts[i] = 0;
        }

        int thread_id = omp_get_thread_num();

        // the count is stored in counts[thread_id]
        int ret = PAPI_stop(event_sets[thread_id], thread_counts);
        if (ret != PAPI_OK) {
            fprintf(stderr, "Error stopping count: %s\n", PAPI_strerror(ret));
        }

        PAPI_cleanup_eventset(event_sets[thread_id]);
        PAPI_destroy_eventset(&event_sets[thread_id]);

// Accumulates all the counts from each thread
#pragma omp critical
        {
            for (int i = 0; i < N_SIMUL_EVENTS; i++) {
                totals[i] += thread_counts[i];
            }
        }
    }

    for (int i = 0; i < N_SIMUL_EVENTS; i++) {
        cout << events.at(i + first_event_id) << ": ";
        cout << totals[i];
        cout << endl;
        // cout << ",";
    }
}

void fillMat(double *mat, int len) {
#pragma omp parallel for
    for (int i = 0; i < len; i++) {
        mat[i] = (double)(i);
    }
}

void get_events() {
    // Native events
    ifstream events_file(native_events_path);
    if (!events_file.is_open()) {
        std::cerr << "Unable to read native events" << std::endl;
        exit(1);
    }

    string line;
    while (getline(events_file, line)) {
        if (!line.empty()) {
            events.push_back(line);
        }
    }

    events_file.close();

    n_events = events.size();
}

int main(int argc, char **argv) {
    if (argc != 4) {
        cout << "usage: " << argv[0] << " <m> <n> <NHRS>" << endl;
        return -1;
    }

    // Setup for PAPI
    get_events();

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

    double average_time = 0;
    // Get counter data, N_SIMUL_EVENTS at a time
    for (int i = 0; i < n_events; i += N_SIMUL_EVENTS) {
        info = 0;

        work = new double[lwork];

        papi_profile_start(events.at(i));

        double start, end;
        start = omp_get_wtime();

        // Run dgels
        dgels_(&Nchar, &m, &n, &NRHS, A, &lda, B, &ldb, work, &lwork, &info);

        end = omp_get_wtime();
        average_time += end - start;

        papi_profile_end(i);

        delete[] work;
    }
    cout << "Runtime: " << average_time / n_events << endl;

    PAPI_shutdown();
    delete[] A;
    delete[] B;
    free(event_sets);

    return (int)info;
}
