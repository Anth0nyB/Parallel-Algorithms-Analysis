#ifndef PERF_H
#define PERF_H

#define native_events_path "../events/hand_picked_events.txt"

#include <string>
#include <vector>

#include "papi.h"

int get_events(std::vector<std::string>& events);

bool papi_profile_start(int* event_sets, std::string event_name);

long long papi_profile_end(int n_threads, int* event_sets, std::string event_name);

void papi_clear_counts(int* event_sets);

#endif