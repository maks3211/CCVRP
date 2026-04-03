#pragma once 
#include "route.h"
#include "move.h"
#include "penalty_functions.h"
#include "utils.h"



//wyznacza gain dla 1_insertion z metody hybrid 
double calculate_gain_1_insertion_hybrid(std::vector<Route>& solution,	Move& move, double avg_cost, double beta = 0.05, double gamma = 0.6);

double calculate_gain_1_1_exchange_hybrid(std::vector<Route>& solution, Move& move, double avg_cost, double beta = 0.05, double gamma = 0.6);
double calculate_gain_2_insertion_hybrid(std::vector<Route>& solution, Move& move, double avg_cost, double beta = 0.05, double gamma = 0.6);
double calculate_gain_2_opt_hybrid(std::vector<Route>& solution, Move& move, double avg_cost, double beta = 0.05, double gamma = 0.6);
double calculate_gain_2_opt_prim_hybrid(std::vector<Route>& solution, Move& move, double avg_cost, double beta = 0.05, double gamma = 0.6);
std::vector<Move>  calculate_gain_cross_tail_hybrid(std::vector<Route>& solution, Move& move, double avg_cost, double beta = 0.05, double gamma = 0.6);