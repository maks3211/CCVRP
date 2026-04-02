#include "hybridAvnsLns.h"

HybridAvnsLns::HybridAvnsLns(CVRPInstance instance, int num_vehicles, IO_handlerV2::IO_handler io_handlers_v2, hybridAvnsLnsConfig config) : instance(instance), io_handlers_v2(io_handlers_v2), num_vehicles(num_vehicles), config(config)
{

}

std::vector<Route> HybridAvnsLns::construct_intial_solution()
{

	std::vector<DistanceInfo> distances_from_depot = get_all_distances(instance.depot_id - 1, instance);

	std::sort(distances_from_depot.begin(), distances_from_depot.end());
	std::vector<Route> routes;

	//Utworzenie tras/ pojazdow
	for (int i = 0; i < num_vehicles; ++i) {
		routes.push_back(Route(i, instance.capacity));  // Pojazd i o odpowiedniej pojemnosci
	}

	std::vector <int> indexes_to_remove;
	//dodanie po kolei najlpesze trasy do kolejnych pojazdow (dodanie pierwszygo klienta do kazdej z trasy)
    for (auto& c : routes)
    {
        if (distances_from_depot.empty()) break;
        auto best = distances_from_depot.front();
        distances_from_depot.erase(distances_from_depot.begin());
        //best second to indeks w oryginalnej tablicy instances z wejsciowymi danymi
        //c.add_customer(instance.nodes[best.second], best.first, 1);

        //szukanie indeksu klienta na podstawie node_id
        auto it = std::find_if(instance.nodes.begin(), instance.nodes.end(),
            [&](const Node& n) { return n.id == best.node_id; });
        if (it != instance.nodes.end()) {
            c.add_customer_at_index(*it, 1, best.distance);
			instance.nodes.erase(it); //usun wstawionego klienta do trasy z listy wszystkich klientów
        }
        else {
            std::cerr << "Nie znaleziono klienta o ID " << best.node_id << "!\n";
        }
        //indexes_to_remove.push_back(best.node_id);
    }

    //usuniecie magazynu z listy klientow
    instance.nodes.erase(instance.nodes.begin());

	int total_customers = instance.nodes.size();



    InsertionResult best_feasible_insertion;
    best_feasible_insertion.cost = std::numeric_limits<double>::infinity();

    //Sprawdzam wszystkie pozycje dla wszystkich klientow i wybieram najlepsze (uwzgledniam pojemnosc)
	//przejscie przez wszystkich klientow

    while (!instance.nodes.empty())
    {
        InsertionResult global_best;
        global_best.cost = std::numeric_limits<double>::infinity();
        global_best.route_id = -1;
        global_best.client_index = -1;
        global_best.place = -1;

        for (int i = 0; i < instance.nodes.size(); i++)
        {
            const Node& client = instance.nodes[i];
                       //szukanie najlepszej trasy dla klienta i
            for (int r = 0; r < routes.size(); r++)
            {
                if (client.demand > routes[r].remaining_capacity) continue;


                InsertionResult insertion = find_best_insertion(routes[r], instance.nodes[i]);
                if (insertion.cost < global_best.cost ||
                    (insertion.cost == global_best.cost && client.id < instance.nodes[global_best.client_index].id))
                {
                    global_best = insertion;
                    global_best.route_id = r;
                    global_best.client_index = i;
                }
            }
        }

        if (global_best.route_id != -1)
        {
            // === NORMALNE WSTAWIENIE ===
            routes[global_best.route_id].add_customer_at_index(
                instance.nodes[global_best.client_index],
                global_best.place + 1,
                global_best.cost);

            instance.nodes.erase(instance.nodes.begin() + global_best.client_index);
        }
        else
        {
            // === PERTURBATION ===
            if (instance.nodes.empty()) break;

            Node problematicClient = instance.nodes[0];     // na razie pierwszy (możesz zmienić na max demand)

            std::cout << "Brak feasible wstawienia → uruchamiam perturbation dla klienta "
                << problematicClient.id << "\n";

            perform_perturbation(routes, problematicClient, instance,total_customers);

            // Klient został już dodany (normalnie lub z penalizacją) → usuwamy go z listy
            if (!instance.nodes.empty())
                instance.nodes.erase(instance.nodes.begin());
        }
    }

	return routes;
}



void HybridAvnsLns::perform_perturbation(std::vector<Route>& routes, Node clientA, CVRPInstance& instance, int total_customers)
{
    const int ts = std::max(5, static_cast<int>(0.01 * total_customers));
    std::vector<int> tabu_until(instance.nodes.size() + 100, 0);
    int iter = 0;
    const int max_iter = 50;
    bool success = false;

    while (iter < max_iter && !success)
    {
        for (size_t r = 0; r < routes.size() && !success; ++r)
        {
            for (size_t pos = 1; pos < routes[r].customers.size() && !success; ++pos)
            {
                Node clientB = routes[r].customers[pos];

                if (tabu_until[clientB.id] > iter) continue;

                // Usuwamy B
                routes[r].remove_customer_at_index(pos);
				
                // Próbujemy wstawić A (forced)
                InsertionResult insA = find_best_insertion(routes[r], clientA);

                double added_cost = calculate_insertion_cost(routes[r], clientA, insA.place).cost;


                routes[r].add_customer_at_index(clientA, insA.place + 1, added_cost);
                tabu_until[clientA.id] = iter + ts;

                    // Teraz wstawiamy B z powrotem
                bool b_inserted = false;
                    for (size_t rr = 0; rr < routes.size() && !b_inserted; ++rr)
                    {
                        if (clientB.demand <= routes[rr].remaining_capacity)
                        {
                            InsertionResult insB = find_best_insertion(routes[rr], clientB);
                            if (insB.feasible)
                            {
                                routes[rr].add_customer_at_index(clientB, insB.place + 1, insB.cost);
                                tabu_until[clientB.id] = iter + ts;
                                b_inserted = true;
                                success = true;
                            }
                        }
                    }

                    if (!b_inserted)
                    {
                        int worst_r = find_route_with_smallest_violation(routes);
                        add_customer_at_index_with_penalty(routes[worst_r],clientB);
                        success = true;
                    }
                
                    if (success)
                        break; 
                    else
                    {
                        routes[r].restore_customer_at(pos, clientB);
                    }
            }
        }
        iter++;
    }

    // Ostateczny fallback
    if (!success)
    {
        std::cout << "[PERTURBATION] Nie udało się znaleźć miejsca → ostateczny fallback\n";
        int r = find_route_with_smallest_violation(routes);
        add_customer_at_index_with_penalty(routes[r],clientA);
    }
}

