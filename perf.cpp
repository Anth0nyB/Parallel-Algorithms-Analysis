#include "perf.h"

#include <omp.h>

#include <fstream>
#include <iostream>

using namespace std;

int get_events(vector<string>& events) {
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

    return events.size();
}

void papi_profile_start(int* event_sets, string event_name) {
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

void papi_profile_end(int* event_sets, vector<string>& events, int first_event_id) {
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
