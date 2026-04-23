#pragma once
#include <iostream>
#include <vector>
#include<algorithm>
#include "route.h"
#include "cvrp_types.h"
#include "move.h"
bool perform_regert_cost_insertion(std::vector<Route>& routes, std::vector<Node>& customers_to_insert);