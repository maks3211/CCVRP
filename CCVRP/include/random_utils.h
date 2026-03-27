#pragma once
#include <utility>
#include <vector>
#include <random>
#include <numeric>
#include <algorithm>
#include "route.h"

extern std::mt19937 rng;

/// <summary>
/// </summary>
/// <param name="s">- Solution to get random client from</param>
/// <returns>Random client (not depot)</returns>
std::pair<int, int> get_random_client(std::vector<Route>& s);



//zwraca indeksy wszystkich tras w losowej kolejnosci
std::vector<int> get_random_route_indexes(std::vector<Route>& s);

int get_random_clients_from_route(Route& route, int num_clients);