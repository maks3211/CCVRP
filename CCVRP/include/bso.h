#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath> 
#include "route.h"
#include "cvrp_types.h"
#include "random_utils.h"
#include "utils.h"
#include "IO_handlerV2.h"
#include "local_search_engine_moves.h"
#include "single_route_improvement.h"
#include "regret_cost_insertion.h"
struct brainConfig {
	int T1 = 500; // 'called single route improvement' iteration number  - wykorzystywane w construct_initial_solution
	double single_route_improvement_margin = 0.01;  //moj parametr wykorzysytwany w single_route_improvement - daje mozliwosc akceptacji rozwiazan ktore sa gorsze od najelpeszego(czyli wejsciowego) o ta wartosc
	double alfa_1 = 0.05; // 3.2.1 - oglnie 0.05 (5%) powoduje bardzo niewielkie zmiany, przy 10% jest juz nieco lepiej, moze to powodowac spradek jakosci,
	//ale przez to wprowadza wieksze zmiany
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
	/// <summary>
	/// 3.2.1 Perturbation on the best-so-far solution 
	/// Removes r1 random customers and inserts them back using regret-cost-insertion
	/// </summary>
	/// <param name="sb"></param>
	/// <returns>spb - new solution, not necessarily better than input sb solution</returns>
	std::vector<Route> perturbation(std::vector<Route>& sb);

};

