#include "neighbourhood_structures.h"

//Wstawiamy dwoch losowych sasiadow na pierwsze pozycje do dwoch losowych tras jezlei jest miejsce, jezlien nie uda sie wstawic do ktorejs z 
//tras to zwracamy bez zmian



std::vector<Route> two_insertion(std::vector<Route>& current_solution)
{
	if (current_solution.size() < 3)
		return current_solution;

	std::vector<int> random_order_routes = get_random_route_indexes(current_solution);
	int random_index = 0;
	int donor_route_index = random_order_routes[random_index];
	Route donor = current_solution[donor_route_index];

	while (donor.customers.size() < 3) {
		if (random_index >= random_order_routes.size() - 1) {
			return current_solution; // brak odpowiedniej trasy, zwracamy bez zmian
		}
		donor = current_solution[random_order_routes[++random_index]];
	}
	random_order_routes.erase(random_order_routes.begin() + random_index); // usuwamy trasê dawcy z listy tras do wyboru


	//losoujemy dwoch sasiadow z losowej trasy
	int donor_first_pos = get_random_clients_from_route(donor, 2);

	Node first_from_donor = donor.customers[donor_first_pos];
	Node second_from_donor = donor.customers[donor_first_pos + 1];

	donor.customers.erase(donor.customers.begin() + donor_first_pos, donor.customers.begin() + donor_first_pos + 2);
	donor.remaining_capacity += first_from_donor.demand + second_from_donor.demand;

	//losujemy dwie trasy odbierajace
	random_index = 0;
	int receiver1_index = random_order_routes[random_index];
	Route receiver1 = current_solution[receiver1_index];
	bool found_receiver = false;
	do 
	{

		if (receiver1.remaining_capacity >= first_from_donor.demand)
		{
			receiver1.customers.insert(receiver1.customers.begin() + 1, first_from_donor);
			receiver1.remaining_capacity -= first_from_donor.demand;		
			random_order_routes.erase(random_order_routes.begin() + random_index); // usuwamy trasê odbieraj¹c¹ z listy tras do wyboru
			found_receiver = true;
			break;
		}
		random_index++;
		if (random_index < random_order_routes.size()) {
			receiver1_index = random_order_routes[random_index];
			receiver1 = current_solution[receiver1_index]; // aktualizujemy receiver1
		}
	} while (random_index < random_order_routes.size());

	if(!found_receiver)
	{
		return current_solution;
	}
	found_receiver = false;
	random_index = 0;
	int receiver2_index = random_order_routes[random_index];
	Route receiver2 = current_solution[receiver2_index];
	do
	{

		if (receiver2.remaining_capacity >= second_from_donor.demand)
		{
			receiver2.customers.insert(receiver2.customers.begin() + 1, second_from_donor);
			receiver2.remaining_capacity -= second_from_donor.demand;
			
			random_order_routes.erase(random_order_routes.begin() + random_index); // usuwamy trasê odbieraj¹c¹ z listy tras do wyboru
			found_receiver = true;
			break;
		}
		random_index++;
		if (random_index < random_order_routes.size()) {
			receiver2_index = random_order_routes[random_index];
			receiver2 = current_solution[receiver2_index]; // aktualizujemy receiver2
		}
	} while (random_index < random_order_routes.size());
	
	if (!found_receiver)
	{
		return current_solution;
	}

	g(donor); // aktualizacja kosztu trasy
	g(receiver1); 
	g(receiver2); 
	std::vector<Route> new_solution = current_solution;
	new_solution[donor_route_index] = donor;	
	new_solution[receiver1_index] = receiver1;
	new_solution[receiver2_index] = receiver2;
	return new_solution;
}

std::vector<Route> two_one_interchange(std::vector<Route>& current_solution)
{
	if (current_solution.size() < 3)
	{
		return current_solution; // brak wystarczaj¹cej liczby tras, zwracamy bez zmian
	}

	std::vector<int> random_order_routes = get_random_route_indexes(current_solution);
	int random_index = 0;
	int donor_route_index = random_order_routes[random_index];
	Route donor = current_solution[donor_route_index];

	while (donor.customers.size() < 3) {
		if (random_index >= random_order_routes.size() - 1) {
			return current_solution; // brak odpowiedniej trasy, zwracamy bez zmian
		}
		donor = current_solution[random_order_routes[++random_index]];
	}
	random_order_routes.erase(random_order_routes.begin() + random_index); // usuwamy trasê dawcy z listy tras do wyboru



	int donor_a_index = get_random_clients_from_route(donor, 1);
	int donor_b_index = get_random_clients_from_route(donor, 1);
	while (donor_a_index == donor_b_index)
	{
		donor_b_index = get_random_clients_from_route(donor, 1);
	}

	//a musi byc wiekszy aby b sie nie przesuwalo po usunieciu
	if (donor_b_index > donor_a_index)
	{
		std::swap(donor_a_index, donor_b_index);
	}


	Node donor_a = donor.customers[donor_a_index];	


	donor.customers.erase(donor.customers.begin() + donor_a_index);
	donor.remaining_capacity += donor_a.demand;


	//losujemy dwie trasy odbierajace
	//wstawienie pierwszego klienta do trasy B  
	random_index = 0;
	int receiver1_index = random_order_routes[random_index];
	Route receiver1 = current_solution[receiver1_index];
	bool found_receiver = false;
	do
	{

		if (receiver1.remaining_capacity >= donor_a.demand)
		{
			receiver1.customers.insert(receiver1.customers.begin() + 1, donor_a);
			receiver1.remaining_capacity -= donor_a.demand;
		
			random_order_routes.erase(random_order_routes.begin() + random_index); // usuwamy trasê odbieraj¹c¹ z listy tras do wyboru
			found_receiver = true;
			break;
		}
		random_index++;
		if (random_index < random_order_routes.size()) {
			receiver1_index = random_order_routes[random_index];
			receiver1 = current_solution[receiver1_index]; // aktualizujemy receiver1
		}
	} while (random_index < random_order_routes.size());

	if (!found_receiver)
	{
		return current_solution;
	}

	//szukamy losowego klienta z trasy C ktorego moge wstawic do trasy A 
	//(czyli w trasie A musi byc miejsce na klienta z trasy C i w trasie C musi byc miejsce na klienta z trasy A)
	found_receiver = false;
	random_index = 0;
	int receiver2_index = random_order_routes[random_index];
	Route receiver2 = current_solution[receiver2_index];
	int random_cllient_from_receiver2_index = get_random_clients_from_route(receiver2, 1);
	Node random_client_from_recevier2 = receiver2.customers[random_cllient_from_receiver2_index];	

	do
	{
		if (receiver2.remaining_capacity + random_client_from_recevier2.demand >= donor.customers[donor_b_index].demand   
			 && donor.remaining_capacity + donor.customers[donor_b_index].demand >= random_client_from_recevier2.demand)
		{
			receiver2.remaining_capacity -= donor.customers[donor_b_index].demand;
			receiver2.remaining_capacity += random_client_from_recevier2.demand;
			donor.remaining_capacity -= random_client_from_recevier2.demand;
			donor.remaining_capacity += donor.customers[donor_b_index].demand;


			std::swap(donor.customers[donor_b_index], receiver2.customers[random_cllient_from_receiver2_index]);			
		
			random_order_routes.erase(random_order_routes.begin() + random_index); // usuwamy trasê odbieraj¹c¹ z listy tras do wyboru
			found_receiver = true;
			break;
		}
		random_index++;
		if (random_index < random_order_routes.size()) {
			receiver2_index = random_order_routes[random_index];
			receiver2 = current_solution[receiver2_index]; // aktualizujemy receiver2
			random_cllient_from_receiver2_index = get_random_clients_from_route(receiver2, 1);
			random_client_from_recevier2 = receiver2.customers[random_cllient_from_receiver2_index];
		}
	} while (random_index < random_order_routes.size());

	if (!found_receiver)
	{
		return current_solution;
	}
	g(receiver1); 
	g(receiver2); // aktualizacja kosztu trasy
	g(donor); // aktualizacja kosztu trasy
	std::vector<Route> new_solution = current_solution;
	new_solution[donor_route_index] = donor;
	new_solution[receiver1_index] = receiver1;
	new_solution[receiver2_index] = receiver2;
	return new_solution;
}



std::vector<Route> segment_reshuffle(std::vector<Route>& current_solution, int nk)
{

	if (current_solution.size() < 1)
		return current_solution;

	std::vector<int> random_order_routes = get_random_route_indexes(current_solution);

	int random_index = 0;
	int donor_route_index = random_order_routes[random_index];
	Route donor = current_solution[donor_route_index];

	if (nk == -1)
	{
		nk = donor.customers.size() - 1; // nie licze bazy
	}
	int min = static_cast<int>(0.5 * nk);
	int max = static_cast<int>(0.7 * nk);

	if (max < 1)
		return current_solution;
	if (min < 1)
		min = 1;

	std::uniform_int_distribution<>dist(min,max);
	int number_of_clients_to_shuffle = dist(rng);


	int shuffle_first_pos = get_random_clients_from_route(donor, number_of_clients_to_shuffle);
	if (shuffle_first_pos == -1)
		return current_solution;
	std::shuffle(donor.customers.begin() + shuffle_first_pos, donor.customers.begin() + shuffle_first_pos + number_of_clients_to_shuffle, rng);
	g(donor);
	std::vector<Route> new_solution = current_solution;
	new_solution[donor_route_index] = donor;
	return new_solution;
}


std::vector<Route> cross_exchange(std::vector<Route>& current_solution, int nj, int nk)
{
	if (current_solution.size() < 1)
		return current_solution;

	std::vector<int> random_order_routes = get_random_route_indexes(current_solution);

	int random_index = 0;
	int donor_route_index = random_order_routes[random_index];
	Route route_j = current_solution[donor_route_index];
	random_order_routes.erase(random_order_routes.begin());
	if (nj == -1)
	{
		nj = route_j.customers.size() - 1; // nie licze bazy
	}
	int min_j = static_cast<int>(0.2 * nj);
	int max_j = static_cast<int>(0.4 * nj);

	if (max_j < 1)
		return current_solution;
	if (min_j < 1)
		min_j = 1;

	std::uniform_int_distribution<>dist(min_j, max_j);
	int number_of_clients_to_shuffle_m1 = dist(rng);
	int shuffle_first_pos_m1 = get_random_clients_from_route(route_j, number_of_clients_to_shuffle_m1);

	int total_demand_m1 = 0;
	
	 
	for (int i = shuffle_first_pos_m1; i < shuffle_first_pos_m1+ number_of_clients_to_shuffle_m1; i++)
	{
		total_demand_m1 += route_j.customers[i].demand;
	}
	int free_capacity_j = route_j.remaining_capacity + total_demand_m1;

	//szukanie drugiej trasy
	bool second_route_found = false;

	do
	{
		int receiver_route_index = random_order_routes[random_index];
		Route route_k = current_solution[receiver_route_index];
		if (nk == -1)
		{
			nk = route_k.customers.size() - 1; // nie licze bazy
		}
		int min_k = static_cast<int>(0.2 * nk);
		int max_k = static_cast<int>(0.4 * nk);
		if (max_k < 1)
		{
			random_index++;
			continue; // szukamy innej trasy k - ta ma za ma³o klientów
		}
		if (min_k < 1)
			min_k = 1;

		std::uniform_int_distribution<>dist(min_k, max_k);
		int number_of_clients_to_shuffle_m2 = dist(rng);
		int shuffle_first_pos_m2 = get_random_clients_from_route(route_k, number_of_clients_to_shuffle_m2);
		int total_demand_m2 = 0;

		for (int i = shuffle_first_pos_m2; i < shuffle_first_pos_m2 + number_of_clients_to_shuffle_m2; i++)
		{
			total_demand_m2 += route_k.customers[i].demand;
		}
		int free_capacity_k = route_k.remaining_capacity + total_demand_m2;

		//zmiana mozliwa - nie naruszam pojemnosci w obu trasach
		if (free_capacity_j >= total_demand_m2 && free_capacity_k >= total_demand_m1)
		{
			//kopia fragmentu do przenisienia z trasy j 
			std::vector<Node> tmp(route_j.customers.begin() + shuffle_first_pos_m1, route_j.customers.begin() + shuffle_first_pos_m1 + number_of_clients_to_shuffle_m1);

			route_j.customers.erase(route_j.customers.begin() + shuffle_first_pos_m1, route_j.customers.begin() + shuffle_first_pos_m1 + number_of_clients_to_shuffle_m1);
			route_j.customers.insert(route_j.customers.begin() + shuffle_first_pos_m1, route_k.customers.begin() + shuffle_first_pos_m2, route_k.customers.begin() + shuffle_first_pos_m2 + number_of_clients_to_shuffle_m2);

			route_k.customers.erase(route_k.customers.begin() + shuffle_first_pos_m2, route_k.customers.begin() + shuffle_first_pos_m2 + number_of_clients_to_shuffle_m2);
			route_k.customers.insert(route_k.customers.begin() + shuffle_first_pos_m2, tmp.begin(), tmp.end());

			route_j.remaining_capacity = free_capacity_j - total_demand_m2;
			route_k.remaining_capacity = free_capacity_k - total_demand_m1;

		
			std::vector<Route> new_solution = current_solution;
			new_solution[donor_route_index] = route_j;
			new_solution[receiver_route_index] = route_k;
			return new_solution;

		}
		else // nie miesci sie w trasach, szukamy innej trasy
		{
			nk = -1; // przy nastepnym losowaniu trasy k losujemy inna liczbe klientow do przeniesienia
			random_index++;
		}
	} while (!second_route_found && random_index < random_order_routes.size());

	return current_solution; // nie znaleziono odpowiedniej trasy k, zwracamy bez zmian	
}


std::vector<Route> head_swap(std::vector<Route>& current_solution)
{
	if (current_solution.size() < 2)
		return current_solution;





}
