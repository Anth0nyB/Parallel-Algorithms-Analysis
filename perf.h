#ifndef PERF_H
#define PERF_H

#define native_events_path "../events/working_events.txt"
#define N_SIMUL_EVENTS 1

#include <string>
#include <vector>

#include "papi.h"

int get_events(std::vector<std::string>& events);

void papi_profile_start(int* event_sets, std::string event_name);

void papi_profile_end(int* event_sets, std::vector<std::string>& events, int first_event_id);

#endif