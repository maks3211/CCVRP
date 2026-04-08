#include "hybridAvnsLns.h"

HybridAvnsLns::HybridAvnsLns(CVRPInstance instance, int num_vehicles, IO_handlerV2::IO_handler io_handlers_v2, hybridAvnsLnsConfig config) : instance(instance), io_handlers_v2(io_handlers_v2), num_vehicles(num_vehicles), config(config)
{

}




std::vector<Route> HybridAvnsLns::construct_intial_solution()
{
    int total_customers = instance.nodes.size() - 1; //wszyscy klieci bez magazynu
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

	
    //WCZESNIEJ TU BYLO int total_customers = instance.nodes.size()


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

            perform_perturbation(routes, problematicClient,total_customers);

            // Klient został już dodany (normalnie lub z penalizacją) → usuwamy go z listy
            if (!instance.nodes.empty())
                instance.nodes.erase(instance.nodes.begin());
        }
    }

	return routes;
}



void HybridAvnsLns::perform_perturbation(std::vector<Route>& routes, Node clientA, int total_customers)
{
    const int ts = std::max(5, static_cast<int>(0.01 * total_customers));
    std::vector<int> tabu_until(total_customers + 100, 0);
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


//Algorithm 1 The AVNS algorithm (Stage 1)
std::vector<Route> HybridAvnsLns::AVNS_stage_one(std::vector<Route>& solution, int total_customers)
{
    std::vector<Route> x = solution;
    std::vector<Route> x_best = x;
    for (int i = 0; i < 6; ++i) {
        scoreLh[i] = 0.0;         
    }
    double lambda = lambda_min;
    
    double x_cost = 0.0;
    double x_best_cost = 0.0;
    x_cost = get_sum_of_route_cost(x);
    x_best_cost = x_cost;

    int numDiv = 1;
    while (numDiv <= maxDiv)
    {
        int p = 1;
        while (p <= p_max)
        {
            std::vector<Route> x_prim = S_p_neighbourhood(p, x);  //linia 5 algorytmu


            double gain[6] = { 0.0 }; //kolejne wartosci gain 
            BestMoves best_improvement_local_search;
            Move best_move; //najlepszy ruch
            int best_move_number = 0;
            double best_gain = -std::numeric_limits<double>::infinity();

            //Linia 6 
            for (int h = 1; h <= h_max; ++h) //przejscie przez wszystkie 6 operatorow local_search 
            {
                best_improvement_local_search = L_h_local_serach(x_prim, h, 1); //1 - bierzemy najlepszy ruch
                gain[h - 1] = best_improvement_local_search.topK[0].gain;

                if (best_improvement_local_search.topK[0].gain > best_gain) // znaleziono lepszy ruch
                {
                    best_move = best_improvement_local_search.topK[0];
                    best_gain = best_move.gain;
                    best_move_number = h;

                }
            }

            //wyznaczenie score dla kazdego Lh  
            for (int i = 0; i < 6; ++i)
            {
                scoreLh[i] += gain[i] / best_gain;
            }

            //linia 7
            //wykonanie najelpeszegu ruchu Lh 
            //wyznaczenie srednioego kosztu rozwiazanie x_prim;
            double avg_solution_cost = 0.0;
            for (int i = 0; i < x_prim.size(); ++i)
            {
                avg_solution_cost += x_prim[i].route_cost;
            }
            avg_solution_cost /= x_prim.size();

            std::vector<Route> x_bis = perform_move(x_prim, best_move, best_move_number, avg_solution_cost); // linia 7 

            //Move or not 
            double x_bis_cost = get_sum_of_route_cost(x_bis);


            if (x_bis_cost < x_cost) //jest poprawa  -- lina 9
            {
                x = x_bis;
                p = 1;
                x_cost = x_bis_cost;
            }
            else //linia 11
            {
                p = p + 1;
            }
        }//linia 14

        if (x_cost < x_best_cost) // linia 15
        {
            x_best = x;
            lambda = lambda_min;
        }
        else
        {
            lambda = lambda + 0.05 * total_customers; // linia 18
        }

        //Diversification: Apply the LNS based on λ to get x~ 
        double avg_solution_cost = 0.0;
        for (int i = 0; i < x.size(); ++i)
        {
            avg_solution_cost += x[i].route_cost;
        }
        avg_solution_cost /= x.size();
        std::vector<Route> x_tylda = lns_diversification(x, lambda, avg_solution_cost, total_customers); //lina 20
        double x_tylda_cost = get_sum_of_route_cost(x_tylda);
        if (x_tylda_cost < x_best_cost) //linia 21
        {
            x_best = x_tylda;
        }

        numDiv = numDiv + 1;
        x = x_tylda;
        x_cost = x_tylda_cost;
    }  


  

    double score_sum = 0.0;
    for (int i = 0; i < 6; ++i)
    {
        score_sum += scoreLh[i];
    }
    //linia 26 


    for (int i = 0; i < 6; ++i)
    {
        probLh[i] = scoreLh[i] / score_sum;
        if (i > 0)
        {
            fLh[i] = fLh[i - 1] + probLh[i];
        }
        else
        {
            fLh[i] = probLh[i];
        }
    }
    fLh[5] = 1.0;


    //CZY TO NALEZY ZWRACAC??
    return x_best;
}






