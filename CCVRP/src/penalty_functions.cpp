#include "penalty_functions.h"




double calculate_penalty_hybrid(double capacity, double new_demand, double average_route_cost, double beta, double gamma)
{
	
	if (new_demand <= capacity)
	{
		return 0; // nie naruszamy pojemnosci
	}
	double max_capacity_violation = (1 + beta) * capacity;
	double zeta = gamma / beta * average_route_cost; // maksymalna dopuszczalna kara za przekroczenie czasu trasy
	double max_component = ((new_demand - capacity) / capacity ) * zeta;

	//η -eta  kara za przekroczenie pojemnosci, obliczana na podstawie wzoru 7 w sekcji 3.4 artykułu 198
	double eta = std::max(0.0, max_component);
	
	return eta;

}
