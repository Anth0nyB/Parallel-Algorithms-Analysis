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

    // Setup for PAPI
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
    LAPACK_INT lwork_probe = -1;

    double *A = new double[lda * n];
    double *tau = new double[m > n ? m : n];

    // Get optimal nb value
    double lwork_opt, nb_opt;
    dgeqrf_(&m, &n, A, &lda, tau, &lwork_opt, &lwork_probe, &info);
    nb_opt = lwork_opt / n;

    // Different values of lwork to test
    double nb_factors[] = {1};
    int n_lwork_vals = sizeof(nb_factors) / sizeof(double);
    LAPACK_INT lwork[n_lwork_vals];
    for (int i = 0; i < n_lwork_vals; i++) {
        lwork[i] = (LAPACK_INT)(n * nb_opt * nb_factors[i]);
    }

    for (int lwk_index = 0; lwk_index < n_lwork_vals; lwk_index++) {
        // cout << n_threads << " " << m / 1000 << "k " << n / 1000 << "k " << lwork[lwk_index] << ", ";
        cout << n_threads << " " << m / 1000 << "k " << n / 1000 << "k ,";

        double avg_time = 0;

        for (int i = 0; i < n_events; ++i) {
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

            info = 0;

            fillMat(A, lda * n);

            double *work = new double[lwork[lwk_index]];

            // Reset event counters to only measure target application
            papi_clear_counts(event_sets);

            avg_time -= omp_get_wtime();
            dgeqrf_(&m, &n, A, &lda, tau, work, &lwork[lwk_index], &info);
            avg_time += omp_get_wtime();

            // Clean up counters and print result
            cout << papi_profile_end(n_threads, event_sets, events.at(i)) << ",";

            delete[] work;
        }

        cout << avg_time / n_events << "," << endl;
    }

    PAPI_shutdown();
    delete[] A;
    delete[] tau;

    return 0;
}