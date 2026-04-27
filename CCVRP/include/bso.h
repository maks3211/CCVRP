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
#include "divergent_operators.h"
struct brainConfig {
	int main_loop_itarations = 4;

	int T1 = 50; // 'called single route improvement' iteration number  - wykorzystywane w construct_initial_solution
	double single_route_improvement_margin = 0.01;  //moj parametr wykorzysytwany w single_route_improvement - daje mozliwosc akceptacji rozwiazan ktore sa gorsze od najelpeszego(czyli wejsciowego) o ta wartosc
	double alfa_1 = 0.05; // 3.2.1 - oglnie 0.05 (5%) powoduje bardzo niewielkie zmiany, przy 10% jest juz nieco lepiej, moze to powodowac spradek jakosci,
	//ale przez to wprowadza wieksze zmiany

	//3.3 The divergent operation
	int N = 150;// "to generate N new solutions where N is a parameter"
	double alfa_2 = 0.05;
	int T2 = 5; // Liczba iteracji divergent operation dzilajacej na podproblemie
	

};

class BrainStormOptimalization
{
	Result result;
	IO_handlerV2::IO_handler io_handlers_v2;
	int L = 0; // //3.2.2 - wplywa na ilosc tras w podproblemie 
public:
	CVRPInstance instance;
	brainConfig config;
	int num_vehicles;
	int num_of_customers;

	std::vector<double> cost_progress;
	std::vector<double> divergent_cost_progress;
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
	std::vector<Route> perturbation(std::vector<Route>& sb, double alfa);

	/// <summary>
	/// Degree of farness and nearness - forumula (3) 
	/// </summary>
	/// <param name="R1"></param>
	/// <param name="R2"></param>
	/// <returns></returns>
	double compute_T(const Route& R1, const Route& R2);

	/// <summary>
	/// Perform divergent operation from 3.3. input spb will contain best solution obtain by procedure
	/// </summary>
	/// <param name="spb"></param>
	/// <returns>True - if better solution found
	///			 False - if spb is the same as original</returns>
	bool divergent_operation(std::vector<Route>& spb);


	void local_search(std::vector<Route>& routes);

	std::vector<std::vector<Route>> decomposition(std::vector<Route> &routes);
};

