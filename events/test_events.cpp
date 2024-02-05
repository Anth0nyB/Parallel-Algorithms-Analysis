#include <omp.h>

#include <fstream>
#include <iostream>
#include <vector>

#include "papi.h"

#define LAPACK_INT long int
#define events_path "testing_events.txt"

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

int get_events(vector<string> &events) {
    // Native events
    ifstream events_file(events_path);
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

    return events.size();
}

void profile_start(int *event_sets, vector<string> &events) {
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

        for (int i = 0; i < events.size(); i++) {
            ret = PAPI_add_named_event(event_sets[thread_id], events.at(i).c_str());
            if (ret != PAPI_OK) {
                fprintf(stderr, "Error adding event \"%s\": %s\n", events.at(i).c_str(), PAPI_strerror(ret));
            }
        }

        PAPI_reset(event_sets[thread_id]);
        ret = PAPI_start(event_sets[thread_id]);
        if (ret != PAPI_OK) {
            fprintf(stderr, "Error starting: %s\n", PAPI_strerror(ret));
        }
    }
}

void profile_end(int *event_sets, vector<string> &events) {
    // Counts of the events being measured
    long long totals[events.size()];
    for (int i = 0; i < events.size(); i++) {
        totals[i] = 0;
    }

// Each thread stops counting event and cleans up
#pragma omp parallel
    {
        long long counts[events.size()];
        for (int i = 0; i < events.size(); i++) {
            counts[i] = 0;
        }

        int thread_id = omp_get_thread_num();

        int ret = PAPI_stop(event_sets[thread_id], counts);
        if (ret != PAPI_OK) {
            fprintf(stderr, "Error stopping count: %s\n", PAPI_strerror(ret));
        }

        PAPI_cleanup_eventset(event_sets[thread_id]);
        PAPI_destroy_eventset(&event_sets[thread_id]);

// Accumulates all the counts from each thread
#pragma omp critical
        {
            for (int i = 0; i < events.size(); i++) {
                totals[i] += counts[i];
            }
        }
    }

    // cout << event_name << ",\"";
    for (int i = 0; i < events.size(); i++) {
        cout << totals[i];
        cout << ",";
    }
    // cout << counts[n_threads - 1];
    // cout << "\"," << endl;
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
    fillMat(C, m * n, 0);

    cout << "starting" << endl;
    profile_start(event_sets, events);
    cout << "done" << endl;

    double start, end;
    start = omp_get_wtime();

    dgemm_(&Nchar, &Nchar, &m, &n, &k, &alpha, A, &lda, B, &ldb, &beta, C, &ldc);

    end = omp_get_wtime();
    average_time += end - start;

    profile_end(event_sets, events);

    cout << "Runtime: " << average_time / n_events << endl;

    PAPI_shutdown();
    delete[] A;
    delete[] B;
    delete[] C;

    return 0;
}
