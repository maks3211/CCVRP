#include "regret_cost_insertion.h"
//przekazuje vector klientow do wstawinia oraz wektor tras do ktorych ma zostac wykonanie wstawienie
//zmiana jest odrazu widoczna w przekazanym routes
//customers_to_insert nie jest modyfikowane
void perform_regert_cost_insertion(std::vector<Route>& routes,std::vector<Node>& customers_to_insert)
{
	double best_delta_cost = 0.0, second_best_delta_cost = 0.0; // przechowuje dwa najlepsze koszty wstawienia (te ktore powoduja najmnieszy wzrost kosztu)
	double delta_f = 0.0; // regret value

	std::vector<Node> to_insert = customers_to_insert;
	//<-instance = customers_to_insert
	while (!to_insert.empty())
	{
		double maximum_regret_cost = -1.0;
		Move best_move; // Obiekt przechowuj¹cy: Node index, Route index, Position, delta_G, delta_T
		double delta_G = 0.0, delta_T = 0.0;

		for (int i = 0; i < to_insert.size(); ++i)
		{
			Node& new_client = to_insert[i];


			// Najlepsze wyniki DLA TEGO KONKRETNEGO KLIENTA (z roznych tras)
			double c1 = 1e18; // best delta_total
			double c2 = 1e18; // second best delta_total

			// Pomocnicze zmienne do zapamiêtania parametrow najlepszego wstawienia klienta i
			int temp_best_route = -1;
			int temp_best_pos = -1;
			double temp_best_dG = 0.0;
			double temp_best_dT = 0.0;

			for (int r = 0; r < routes.size(); ++r)
			{
				if (routes[r].remaining_capacity < new_client.demand) continue;
				// Szukamy najlepszej pozycji W TEJ TRASIE r
				for (int p = 1; p <= routes[r].customers.size(); ++p)
				{
					double current_dG, current_dT;
					routes[r].calculate_insertion_cost(p, new_client, current_dG, current_dT);
					// Sprawdzamy, czy to najlepsze wstawienie dla tego klienta ogolnie
					if (current_dT < c1) 
					{
						// Stary c1 staje siê drugim najlepszym (z innej trasy)
						// tylko jesli stara najlepsza trasa byla inna niz obecna
						if (r != temp_best_route) 
						{
							c2 = c1;
						}
						c1 = current_dT;
						temp_best_route = r;
						temp_best_pos = p;
						temp_best_dG = current_dG;
						temp_best_dT = current_dT;
					}
					// Jesli jest gorsze od c1, ale lepsze od c2 I pochodzi z innej trasy
					else if (current_dT < c2 && r != temp_best_route)
					{
						c2 = current_dT;
					}
				}
			}
			// Po sprawdzeniu wszystkich tras dla klienta i, obliczamy jego zal
		   // Jesli klient nie ma opcji wstawienia do zadnej trasy, c1 i c2 zostan¹ 1e18
			double current_regret = (c2 >= 1e18) ? c1 : (c2 - c1);

			// Szukamy klienta z MAKSYMALNYM zalem sposrod wszystkich klientow
			if (current_regret > maximum_regret_cost && temp_best_route != -1) {
				maximum_regret_cost = current_regret;
				best_move.from_pos = i; // indeks w to_insert.nodes
				best_move.to_route = temp_best_route;
				best_move.to_pos = temp_best_pos;
				delta_G = temp_best_dG;
				delta_T = temp_best_dT;
			}
		}

		// WYKONANIE NAJLEPSZEGO RUCHU
		if (maximum_regret_cost != -1.0) {
			Node chosen_node = to_insert[best_move.from_pos];
			Route& r = routes[best_move.to_route];

			//r.customers.insert(r.customers.begin() + best_move.to_pos, chosen_node); //wstaw klienta
			r.add_customer_at_index(chosen_node, best_move.to_pos, 0.0, false);
			r.update_arrival_times(best_move.to_pos, chosen_node, delta_G, delta_T);


			// Usuwamy klienta z listy nieobs³u¿onych
			to_insert.erase(to_insert.begin() + best_move.from_pos);
		}
		else {
			std::cout << "NIE UDALO SIE WSTAWIC KLIENTA - NIE MA MIEJSCA";
			break;
		}


	}

}