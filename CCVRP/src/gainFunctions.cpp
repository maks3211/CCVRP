#include "gainFunctions.h"


double calculate_gain_1_insertion_gain_hybrid(std::vector<Route>& solution, Move& move, double avg_cost, double beta, double gamma) {
    Route& r_from = solution[move.from_route];
    Route& r_to = solution[move.to_route];
    
	Node& client_to_move = r_from.customers[move.from_pos];
    //nowa pojemnosc trasy po wykonaniu przeniesienia 
    int new_used_capacity = r_from.initial_capacity - r_from.remaining_capacity - move.moved_capacity_from;
    double delta_eta_k = r_from.penatly_eta - calculate_penalty_hybrid(r_from.initial_capacity, new_used_capacity, avg_cost, beta, gamma);
   
    double delta_C_k = r_from.Z1[move.from_pos] - 0 + delta_eta_k;
    new_used_capacity = r_to.initial_capacity - r_to.remaining_capacity + move.moved_capacity_from;

    //warunek (5) za mocne przekorczenie kosztu
    if (new_used_capacity > (1 + beta) * r_to.initial_capacity)
    {
		return -std::numeric_limits<double>::infinity(); 
    }
    double delta_eta_l = r_to.penatly_eta - calculate_penalty_hybrid(r_to.initial_capacity, new_used_capacity, avg_cost, beta, gamma);;
    double delta_C_l = 0 - r_to.calculate_cost_variation_of_inserting_customer(1,move.to_pos, client_to_move) + delta_eta_l;
	double gain = delta_C_k + delta_C_l;
    return gain;
}



double calculate_gain_1_1_exchange_gain_hybrid(std::vector<Route>& solution, Move& move, double avg_cost, double beta, double gamma)
{
    Route& r_from = solution[move.from_route];
    Route& r_to = solution[move.to_route];

    Node& client_from = r_from.customers[move.from_pos];
    Node& client_from = r_to.customers[move.to_pos];

    int demand_from = move.moved_capacity_from; // pojemnosc klienta przenosznego z trasy 'from' do trasy 'to'
	int demand_to = move.moved_capacity_to; // pojemnosc klienta przenoszonego z trasy 'to' do trasy 'from'

    //to continue - jest w gemini - szukac pod 2. Implementacja funkcji Gain dla 1-1 Exchange


    return 0.0;
}




