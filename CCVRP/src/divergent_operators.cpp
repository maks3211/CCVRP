#include "divergent_operators.h"


int first_operator(std::vector<Route>& solution, Move move)
{



	Route& r1 = solution[move.from_route];
	Route& r2 = solution[move.to_route];

	Node& c1 = r1.customers[move.from_pos];
	Node& c2 = r2.customers[move.to_pos];

	double demand1 = c1.demand;
	double demand2 = c2.demand;

	if (r1.remaining_capacity + demand1 - demand2 < 0)
	{
		return -1;
	}
	if (r2.remaining_capacity + demand2 - demand1 < 0)
	{
		return -1;  // nie ma miesjca
	}
	std::vector<double> next_arrival_times1(r1.customers.size());
	std::vector<double> next_arrival_times2(r2.customers.size());
	double new_cost1 = calculate_virtual_exchange_cost(r1, move.from_pos, c2, next_arrival_times1);
	double new_cost2 = calculate_virtual_exchange_cost(r2, move.to_pos, c1, next_arrival_times2);

	double old_total = r1.route_cost + r2.route_cost;
	double new_total = new_cost1 + new_cost2;

	if (new_total < old_total)
	{
		std::swap(c1,c2);
		r1.arrival_times = next_arrival_times1;
		r1.route_cost = new_cost1;
		r1.remaining_capacity = r1.remaining_capacity + demand1 - demand2; 

		r2.arrival_times = next_arrival_times2;
		r2.route_cost = new_cost2;
		r2.remaining_capacity = r2.remaining_capacity + demand2 - demand1;
		return 1; // jest poprawa zwraca 1
	}

	return 0; // brak poprawy
}

int second_operator(std::vector<Route>& solution, Move move)
{
	Route& r1 = solution[move.from_route];
	Route& r2 = solution[move.to_route];

	int tail_1_length = r1.customers.size() - move.from_pos;
	int demand_1 = 0;
	for (int i = move.from_pos; i < r1.customers.size(); ++i)
	{
		demand_1 += r1.customers[i].demand;
	}

	int tail_2_length = r2.customers.size() - move.to_pos;
	int demand_2 = 0;
	for (int i = move.to_pos; i < r2.customers.size(); ++i)
	{
		demand_2 += r2.customers[i].demand;
	}

	if (r1.remaining_capacity + demand_1 - demand_2 < 0 || r2.remaining_capacity + demand_2 - demand_1 < 0) 
	{
		return -1; // nie ma miejsca
	}

	std::vector<double> next_times1, next_times2;
	double new_cost1 = calculate_cross_path_cost(r1, r2, move.from_pos, move.to_pos, next_times1);
	double new_cost2 = calculate_cross_path_cost(r2, r1, move.to_pos, move.from_pos, next_times2);
	double old_sum_cost = r1.route_cost + r2.route_cost;
	double new_sum_cost = new_cost1 + new_cost2;

	if (new_sum_cost < old_sum_cost)
	{
		std::vector<Node> tail_1(r1.customers.begin() + move.from_pos, r1.customers.end());
		std::vector<Node> tail_2(r2.customers.begin() + move.to_pos, r2.customers.end());

		r1.customers.erase(r1.customers.begin() + move.from_pos, r1.customers.end());
		r2.customers.erase(r2.customers.begin() + move.to_pos, r2.customers.end());

		r1.customers.insert(r1.customers.end(), std::make_move_iterator(tail_2.begin()), std::make_move_iterator(tail_2.end()));
		r2.customers.insert(r2.customers.end(), std::make_move_iterator(tail_1.begin()), std::make_move_iterator(tail_1.end()));

		r1.remaining_capacity += (demand_1 - demand_2);
		r1.route_cost = new_cost1;
		r1.arrival_times = next_times1;

		r2.remaining_capacity += (demand_2 - demand_1);
		r2.route_cost = new_cost2;
		r2.arrival_times = next_times2;
		return 1;
	}
	return 0;
}