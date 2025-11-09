#pragma once
#include <vector>
#include <iostream>
#include "route.h"
#include "cvrp_types.h"
#include "random_utils.h"
#include "utils.h"
#include "IO_handlerV2.h"

struct SkewedVNSConfig {
	int block_size = 3;
	int delta_alfa = 5;
	int f_alfa = 10;
	int SVNS_max_no_improve = 36;
};


class Skewed_VNS
{
	//std::vector<Route> result;
	Result result; // Result of the algorithm
	IO_handlerV2::IO_handler io_handlers_v2;
	
public:
	CVRPInstance instance; // Instance of the CVRP problem
	SkewedVNSConfig config;
	int num_vehicles; // Number of vehicles in the instance
	

	Skewed_VNS(CVRPInstance instance, int num_vehicles ,SkewedVNSConfig config = {});
	
	void run();
	const Result& get_result() const;

	std::vector<Route> constructive_heurestic();

	/// <summary>  
	/// N1 - Simple insert 
	/// N2 - block insert
	/// N3 - simple permute - swapping two customers
	/// All performed intra/ inter route
	/// May violate vehicle capacity constraints	 
	/// </summary>
	///  <param name="s">- Solution to permute</param>
	///  <param name="neighborhood">- Neighborhood type allowed vales 1,2 or 3</param>
	///  <param name="block_size">- N2 block size 3 by default</param>
	///  <returns>First improved solution</returns>
	std::vector<Route> N(std::vector<Route>& s, int neighborhood);

	/// <summary>Variable neighborhood descent metaheuristic. Used for finding a local optimum.
	/// </summary>
	/// <param name="s_initial">- Solution to check</param>
	/// <returns>Best improved solution</returns>
	std::vector<Route> VND(std::vector<Route>& s_initial);


	/// <summary>Generates a solution randomly in p th procedure.
	/// P1 - Move Insert In - inserts random customer in best position inside the route
	/// P2 - Move Insert Out - inserts random customer in best position outside the route
	/// P3 - Move Insert Out - swaps two random customers and then looking for best postion for first of them
	/// Cannot violate vehicle capacity constraints	
	/// </summary>
	/// <param name="s">- Solution to perturbate</param>
	/// <param name="procedure">- Procedure type allowed vales 1,2 or 3</param>
	/// <returns>Best solution from p th procedure</returns>
	std::vector<Route> P(std::vector<Route>& s, int procedure);


	/// <summary>
	/// Distance function between s1 and s2
	/// </summary>
	/// <param name="s1">- Solution one</param>
	/// <param name="s2">- Solution two</param>
	/// <param name="alfa">- Scaling factor 5 by default</param>
	/// <returns>Distance between s1 and s2</returns>
	double delta(std::vector<Route>& s1, std::vector<Route>& s2);


	/// <summary>
	/// SVNS allows exploration of solution areas far from the current solution to escape local optima traps.
	/// It focuses on finding solutions close to the best known but sufficiently different,
	/// enabling the search to discover better solutions in less explored regions of the solution space.
	/// </summary>
	/// <param name="solution">- Initial solution</param>
	/// <param name="k_max">- Maximum neighborhood structure</param>
	/// <returns>Best find solution using SVNS heurestic</returns>
	std::vector<Route> SVNS(std::vector<Route>& solution, int k_max = 3);

	/// <summary>
	/// Solution evaluation function, taking into account the penalty for exceeding the load capacity
	/// </summary>
	/// <param name="s">- Soloution to check</param>
	/// <param name="alfa">- Importance of exceeding the load capacity - passed by config.f_alfa</param>
	/// <param name= "changed_routes">- If only some routes changed, pass their indices to avoid recalculating the cost of unchanged routes</param>
	/// <returns>Solution evaluation</returns>
	double f(std::vector<Route>& s, const std::vector<int>& changed_routes = {}, bool test = false);
};

