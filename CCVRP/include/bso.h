#pragma once
#include <iostream>
#include <vector>
#include<algorithm>
#include "route.h"
#include "cvrp_types.h"
#include "random_utils.h"
#include "utils.h"
#include "IO_handlerV2.h"
#include "local_search_engine_moves.h"
#include "single_route_improvement.h"
struct brainConfig {
	int T1 = 500; // 'called single route improvement' iteration number  - wykorzystywane w construct_initial_solution
	int param_b = 20;
};

class BrainStormOptimalization
{
	Result result;
	IO_handlerV2::IO_handler io_handlers_v2;

public:
	CVRPInstance instance;
	brainConfig config;
	int num_vehicles;
	int num_of_customers;
	BrainStormOptimalization(CVRPInstance instance, int num_vehicles, IO_handlerV2::IO_handler io_handlers_v2, brainConfig = {});

	std::vector<Route> construct_initial_solution();
	void run();
	const Result& get_result() const;
};

