#include <omp.h>

#include <algorithm>
#include <iostream>
#include <vector>

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
        std::cout << "usage: " << argv[0] << " <m> <n>" << endl;
        return -1;
    }

    // Initial setup for PAPI
    int ret = PAPI_library_init(PAPI_VER_CURRENT);
    if (ret != PAPI_VER_CURRENT) {
        fprintf(stderr, "Error initializing PAPI: %s\n", PAPI_strerror(ret));
        return -1;
    }
    int n_threads = omp_get_max_threads();
    int *event_sets = (int *)malloc(n_threads * sizeof(int));

    std::vector<std::string> events;
    int n_events = get_events(events);

    // Setup for dgeqrf
    LAPACK_INT m = atoi(argv[1]);
    LAPACK_INT n = atoi(argv[2]);
    LAPACK_INT lda = m;
    LAPACK_INT info = 0;
    LAPACK_INT lwork_probe = -1;  // Value of -1 tells dgeqrf to calc optimal lwork

    double *A = new double[lda * n];
    double *tau = new double[m > n ? m : n];

    // Get optimal lwork value
    double lwork_opt;
    dgeqrf_(&m, &n, A, &lda, tau, &lwork_opt, &lwork_probe, &info);
    LAPACK_INT lwork = (LAPACK_INT)lwork_opt;

    std::cout << n_threads << " " << m / 1000 << "k " << n / 1000 << "k " << lwork << ", ";
    // cout << n_threads << " " << nb_factors[nb_index] << " " << m / 1000 << "k " << n / 1000 << "k ,";

    double avg_time = 0;

    // Run dgeqrf for each event
    // Only measure one event at a time to ensure hardware counters are capable
    for (int i = 0; i < n_events; ++i) {
        // Run dgeqrf mutliple times for each event and get average count
        vector<long long> trials;
        for (int trial = 0; trial < 5; ++trial) {
            // Sometimes events fail to add on one thread, so give a few tries if this is the case
            int attempts = 0;
            while (attempts < 10) {
                if (!papi_profile_start(event_sets, events.at(i))) {
                    papi_profile_end(n_threads, event_sets, events.at(i));
                } else
                    break;
                attempts++;
            }
            if (attempts == 10) {
                fprintf(stderr, "Failed to measure event: %s", events.at(i));
                continue;
            }

            // Reset some parameters of dgeqrf
            info = 0;

            fillMat(A, lda * n);

            double *work = new double[lwork];

            // Reset event counters to only measure target application
            papi_clear_counts(event_sets);

            avg_time -= omp_get_wtime();
            dgeqrf_(&m, &n, A, &lda, tau, work, &lwork, &info);
            avg_time += omp_get_wtime();

            // Clean up counters and print result
            trials.push_back(papi_profile_end(n_threads, event_sets, events.at(i)));

            delete[] work;
        }

        // Get the mean count for the event, ignoring the highest and lowest counts as outliers
        std::sort(trials.begin(), trials.end());
        long long avg_count = 0;
        for (int j = 1; j < 4; ++j) {
            avg_count += trials.at(j);
        }
        avg_count /= 3;

        std::cout << avg_count << ",";
    }

    std::cout << avg_time / (n_events * 5) << "," << endl;

    PAPI_shutdown();
    delete[] A;
    delete[] tau;

    return 0;
}