#pragma once

#include <vector>
#include <algorithm>
#include "route.h"
/// <summary>
/// 
/// </summary>
/// <param name="capacity">Q - total space</param>
/// <param name="new_demand"></param>
/// <param name="beta">controlls max % of capacity violation </param>
/// <param name="average_route_cost">Avarage route cost (avarege of all routes in solution)</param>
/// <param name="gamma">contols max % of total time increase for new, violated route</param>
/// <returns></returns>
double calculate_penalty_hybrid(double capacity, double new_demand, double average_route_cost, double beta, double gamma);
