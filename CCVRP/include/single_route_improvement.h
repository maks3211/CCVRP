#pragma once
#include <vector>
#include <algorithm>
#include "move.h"	
#include "route.h"	
#include "random_utils.h"
#include "local_search_engine_moves.h"
struct segment_last_index
{
	int i, j, k, l;
};

void single_route_improvement(Route& route, int iterations);

void perform_3_opt(Route& route, int i, int j, int k);
void perform_4_opt(Route& route, int i, int j, int k, int l);