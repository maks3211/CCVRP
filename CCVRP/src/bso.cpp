#include "bso.h"



void check_costs(std::vector<Route>& routes, int op) {
    double total_cost = 0.0;
    std::cout << ".";
    for (int roz = 0; roz < routes.size(); ++roz)
    {
        //std::cout << "\n#" << roz << " Koszt z route_cost : " << routes[roz].route_cost;
        double my_cost = 0.0;
        for (int i = 0; i < routes[roz].customers.size(); ++i)
        {
            my_cost += routes[roz].arrival_times[i];

        }
        double przeliczone = g(routes[roz]);
        
        //std::cout << "\n#" << roz << " Koszt z arrival_times : " << my_cost;
       //std::cout << "\n#" << roz << " Przeliczony  : " << przeliczone;

        total_cost += my_cost;


        double eps = 1e-6;

        bool equal_ab = std::abs(routes[roz].route_cost - my_cost) <= eps;
        bool equal_ac = std::abs(routes[roz].route_cost - przeliczone) <= eps;

        std::cout << std::fixed << std::setprecision(3);
        if (!(equal_ab && equal_ac))
        {
            std::cout << std::fixed << std::setprecision(3)
                << "\n!!!!!??????!!!!!!JEST ROZNACA op: "<<op <<"\n#" << roz << " Koszt z route_cost : " << routes[roz].route_cost
                << "\n#" << roz << " Koszt z arrival_times : " << my_cost
                << "\n#" << roz << " Przeliczony  : " << przeliczone
                << std::endl;
        }
    }
    
}

BrainStormOptimalization::BrainStormOptimalization(CVRPInstance instance, int num_vehicles, IO_handlerV2::IO_handler io_handlers_v2, brainConfig config) : instance(instance), io_handlers_v2(io_handlers_v2), num_vehicles(num_vehicles), config(config)
{
    num_of_customers = instance.nodes.size() - 1;
}


std::vector<Route> BrainStormOptimalization::construct_initial_solution()
{
    //1. Najpierw wstawiam po jednym, najblizszym do magazynu kliencie
    std::vector<DistanceInfo> distances_from_depot = get_all_distances(instance.depot_id - 1, instance);

    std::sort(distances_from_depot.begin(), distances_from_depot.end());
    std::vector<Route> routes;

    //Utworzenie tras/ pojazdow - magazyn dodawany jest w konstruktorze routes 
    for (int i = 0; i < num_vehicles; ++i) {
        routes.push_back(Route(i, instance.capacity));  // Pojazd i o odpowiedniej pojemnosci
        routes[i].cumulative_costs.push_back(0.0);  //koszt skumulowany dojazdu do magazynu = 0
        routes[i].arrival_times.push_back(0.0);
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
            c.cumulative_costs.push_back(best.distance); //zapisanie kosztu skumulowanego dla pierwszego klienta - w praktyce jest to poprstu dystnas 0->A
            c.arrival_times.push_back(best.distance);
            instance.nodes.erase(it); //usun wstawionego klienta do trasy z listy wszystkich klientów
        }
        else {
            std::cerr << "Nie znaleziono klienta o ID " << best.node_id << "!\n";
        }
    }

    //usuniecie magazynu z listy klientow
    instance.nodes.erase(instance.nodes.begin());
    //Teraz instance.nodes ma niewstawionych dotychczas klientow

     //Procedura regret-cost-insertion
    double best_delta_cost, second_best_delta_cost = 0.0; // przechowuje dwa najlepsze koszty wstawienia (te ktore powoduja najmnieszy wzrost kosztu)
    double delta_f = 0.0; // regret value

    //delta_G_plus i delta_total to wartosci ktore sa zwracane przez calculate_insertion_cost s
    //delta_g_plus = 0.0; delta_toatal = 0.0;
    // 1. route calculate_insertion_cost(int insert_pos, const Node& u, double& delta_G_plus, double& delta_total)
    // 2. wstaw klienta
    // 3. update_arrival_times



    while (!instance.nodes.empty())
    {
        double maximum_regret_cost = -1.0;
        Move best_move; // Obiekt przechowuj¹cy: Node index, Route index, Position, delta_G, delta_T
        double delta_G = 0.0, delta_T = 0.0;
        for (int i = 0; i < instance.nodes.size(); ++i)
        {
            Node& new_client = instance.nodes[i];

            // Najlepsze wyniki DLA TEGO KONKRETNEGO KLIENTA (z ró¿nych tras)
            double c1 = 1e18; // best delta_total
            double c2 = 1e18; // second best delta_total

            // Pomocnicze zmienne do zapamiêtania parametrów najlepszego wstawienia klienta i
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

                    // Sprawdzamy, czy to najlepsze wstawienie dla tego klienta ogólnie
                    if (current_dT < c1) {
                        // Stary c1 staje siê drugim najlepszym (z innej trasy)
                        // tylko jeœli stara najlepsza trasa by³a inna ni¿ obecna
                        if (r != temp_best_route) {
                            c2 = c1;
                        }
                        c1 = current_dT;
                        temp_best_route = r;
                        temp_best_pos = p;
                        temp_best_dG = current_dG;
                        temp_best_dT = current_dT;
                    }
                    // Jeœli jest gorsze od c1, ale lepsze od c2 I pochodzi z innej trasy
                    else if (current_dT < c2 && r != temp_best_route) {
                        c2 = current_dT;
                    }
                }
            }

            // Po sprawdzeniu wszystkich tras dla klienta i, obliczamy jego ¿al
            // Jeœli klient nie ma opcji wstawienia do ¿adnej trasy, c1 i c2 zostan¹ 1e18
            double current_regret = (c2 >= 1e18) ? c1 : (c2 - c1);

            // Szukamy klienta z MAKSYMALNYM ¿alem spoœród wszystkich klientów
            if (current_regret > maximum_regret_cost && temp_best_route != -1) {
                maximum_regret_cost = current_regret;
                best_move.from_pos = i; // indeks w instance.nodes
                best_move.to_route = temp_best_route;
                best_move.to_pos = temp_best_pos;
                delta_G = temp_best_dG;
                delta_T = temp_best_dT;
            }
        }

        // WYKONANIE NAJLEPSZEGO RUCHU
        if (maximum_regret_cost != -1.0) {
            Node chosen_node = instance.nodes[best_move.from_pos];
            Route& r = routes[best_move.to_route];

            //r.customers.insert(r.customers.begin() + best_move.to_pos, chosen_node); //wstaw klienta
            r.add_customer_at_index(chosen_node, best_move.to_pos, 0.0, false);
            r.update_arrival_times(best_move.to_pos,chosen_node, delta_G, delta_T);
           

            // Usuwamy klienta z listy nieobs³u¿onych
            instance.nodes.erase(instance.nodes.begin() + best_move.from_pos);
        }
        else {
            std::cout << "NIE UDALO SIE WSTAWIC KLIENTA - NIE MA MIEJSCA";
            break;
        }
    }   

    //DEBUGGIN ONLY
    double total_cost = 0.0;
    std::cout << "\n================= ETAP PIERWSZY =================";
    for (int roz = 0; roz < routes.size(); ++roz)
    {
        std::cout << "\n#" << roz << " Koszt z route_cost : " << routes[roz].route_cost;
        double my_cost = 0.0;
        for (int i = 0; i < routes[roz].customers.size(); ++i)
        {
            my_cost += routes[roz].arrival_times[i];

        }
        double przeliczone = g(routes[roz]);

        std::cout << "\n#" << roz << " Koszt z arrival_times : " << my_cost;
        std::cout << "\n#" << roz << " Przeliczony  : " << przeliczone;
        total_cost += my_cost;
    }
    std::cout << "\n================= Calkowity KOSZT : " << total_cost;
    //END FOR TESTING


    //zakonczono regret-cost-insertion
    //Local serach 
    std::cout << "\n\n\n Local serach  ";
    while (true)
    {
        while (perform_first_improvement_2_opt(routes)) //1
        {
            //Stosuj 2-opt tak dlugo az przynosi to poprawe       
                           
        }
        if (perform_first_improvement_exchange(routes)) //2
        {
          // check_costs(routes,2);
            continue; //jest poprawa wroc do 2-opt
        }
        if (perform_first_improvement_cross(routes)) //3 
        {       
            
            continue;
        }
      
        if (perform_first_improvement_relocation(routes)) //4 - zwraca zly koszt
        {
//            check_costs(routes,4);
            continue;
        }
        break;
    }
    std::cout << "\n\n\nETAP PIERWSZY";

    //FOR TESTING
    total_cost = 0.0;
    std::cout << "\n================= ETAP DRUGI =================";
    for (int roz = 0; roz < routes.size(); ++roz)
    {
        std::cout << "\n#" << roz << " Koszt z route_cost : " << routes[roz].route_cost;
        double my_cost = 0.0;
        for (int i = 0; i < routes[roz].customers.size(); ++i)
        {
            my_cost += routes[roz].arrival_times[i];

        }
        double przeliczone = g(routes[roz]);

        std::cout << "\n#" << roz << " Koszt z arrival_times : " << my_cost;
        std::cout << "\n#" << roz << " Przeliczony  : " << przeliczone;
        total_cost += my_cost;
    }
    std::cout << "\n================= Calkowity KOSZT : =======" << total_cost << "=======";
    //END FOR TESTING
 

    //Single route improvemnt - szukaj poprawy dla kazdej trasy z osobna
    for (int i = 0; i < routes.size(); ++i)
    {
        single_route_improvement(routes[i], config.T1);
    }

    //FOR TESTING
    total_cost = 0.0;
    std::cout << "\n================= ETAP TRZECI =================";
    for (int roz = 0; roz < routes.size(); ++roz)
    {
        std::cout << "\n#" << roz << " Koszt z route_cost : " << routes[roz].route_cost;
        double my_cost = 0.0;
        for (int i = 0; i < routes[roz].customers.size(); ++i)
        {
            my_cost += routes[roz].arrival_times[i];

        }
        double przeliczone = g(routes[roz]);

        std::cout << "\n#" << roz << " Koszt z arrival_times : " << my_cost;
        std::cout << "\n#" << roz << " Przeliczony  : " << przeliczone;
        total_cost += my_cost;
    }
    std::cout << "\n ================= Calkowity KOSZT : ======" << total_cost << "=======";
    //END FOR TESTING

    return routes;
}


void BrainStormOptimalization::run()
{
    //TODO
    std::cout << " BRAIN STORM OPTIMALIZATION STARTED";
    std::vector<Route> sb = construct_initial_solution();
    result.routes = sb;
   
}

const Result& BrainStormOptimalization::get_result() const {
    return result;
}





