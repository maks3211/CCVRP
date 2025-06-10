#pragma once
#include <utility>
#include <vector>
#include <random>
#include "route.h"
/// <summary>
/// </summary>
/// <param name="s">- Solution to get random client from</param>
/// <returns>Random client (not depot)</returns>
std::pair<int, int> get_random_client(std::vector<Route>& s);