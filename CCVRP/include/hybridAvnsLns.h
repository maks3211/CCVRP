#pragma once
#include <vector>
#include <iostream>
#include<algorithm>
#include "route.h"
#include "cvrp_types.h"
#include "random_utils.h"
#include "utils.h"
#include "IO_handlerV2.h"
#include "neighbourhood_structures.h"
#include "local_search_engine_moves.h"
#include "lns_diversification_strategy.h"


struct hybridAvnsLnsConfig {
	int maxDiv = 120;
	int maxDiv2 = 100;
};

class HybridAvnsLns
{
	//std::vector<Route> result;
	Result result; // Result of the algorithm
	IO_handlerV2::IO_handler io_handlers_v2;


	

	//tablica z wynikami dla opertow local
	double scoreLh[6] = { 0.0 };
	double probLh[6] = { 0.0 }; //  the probability -  P rob(Lh)
	double fLh[6] = { 0.0 }; // cumulative pprobability F(Lh)

	//Parametry
	// 
	//parmaetry alogorytmu stage 1 
	int p_max = 5; // 3.6 neighbourhood stuctures  jest piec metod 
	int h_max = 6; // Local serach engine - jest 6 operatorow pkt 3.7
	double lambda_min = 0.0; // patrz linia 337 (strona 16)
	double lambda_max = 0.0; // patrz linia 337 (strona 16)

	//int maxDiv = 120; //USTAWIC NA ODPOWIEDNIA WARTOSC  - nie ma nigdzie podanej ile to ma byc - wiec raczej na zasadzie prob i bledow!!!!
	//int maxDiv2 = 100; //Stage 2
	int lcmin = 3;
	int lcmax = 5;
public:
	CVRPInstance instance; // Instance of the CVRP problem
	hybridAvnsLnsConfig config;
	int num_vehicles; // Number of vehicles in the instance

	HybridAvnsLns(CVRPInstance instance, int num_vehicles, IO_handlerV2::IO_handler io_handlers_v2, hybridAvnsLnsConfig config = {});

	//Algorytm punkt 3.2
	std::vector<Route> construct_intial_solution();

	static void perform_perturbation(std::vector<Route>& routes, Node clientA,int total_customers);

	//Algorithm 1 The AVNS algorithm (Stage 1)
	std::vector<Route> AVNS_stage_one(std::vector<Route> &solution, int total_customers);
	//Algorithm 1 The AVNS algorithm (Stage 2)
	std::vector<Route> AVNS_stage_two(std::vector<Route>& solution, int total_customers);


	void run();
	const Result& get_result() const;

	/// <summary>
	/// 
	/// </summary>
	/// <param name="alfa"></param>
	/// <returns>Zwraca numer operatora (nie jego indeks - czyli funkcja zwraca od 1 do 6)</returns>
	int findLhat(double alfa);

};

