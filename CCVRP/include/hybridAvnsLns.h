#pragma once
#include <vector>
#include <iostream>
#include<algorithm>
#include "route.h"
#include "cvrp_types.h"
#include "random_utils.h"
#include "utils.h"
#include "IO_handlerV2.h"


struct hybridAvnsLnsConfig {
	int paramA = 3;
	int paramB = 5;
};

class HybridAvnsLns
{
	//std::vector<Route> result;
	Result result; // Result of the algorithm
	IO_handlerV2::IO_handler io_handlers_v2;


public:
	CVRPInstance instance; // Instance of the CVRP problem
	hybridAvnsLnsConfig config;
	int num_vehicles; // Number of vehicles in the instance

	HybridAvnsLns(CVRPInstance instance, int num_vehicles, IO_handlerV2::IO_handler io_handlers_v2, hybridAvnsLnsConfig config = {});

	//Algorytm punkt 3.2
	std::vector<Route> construct_intial_solution();

	void perform_perturbation(std::vector<Route>& routes, Node clientA, CVRPInstance& instance, int total_customers);


};

