#pragma once

#include "local_search_engine_moves.h"

//operatory 3.3 

//w praktyce jest to exchange z local move, ale trzeba dostoowac aby robic konkretny ruch a nie kazy/ first improvemnt


/// <summary>
/// 
/// </summary>
/// <param name="solution"></param>
/// <param name="move"></param>
/// <returns> 0 - if no improvemnt, input solution not changed
///			  1 - if move improvement input
///			  -1 - if no space </returns>
int first_operator(std::vector<Route>& solution, Move move);

//Cross
int second_operator(std::vector<Route>& solution, Move move);

