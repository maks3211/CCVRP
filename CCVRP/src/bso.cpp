#include "bso.h"



bool check_costs(std::vector<Route>& routes, int op) {
    double total_cost = 0.0;
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
            
            return true;
        }
    }
    return false;
    
}

BrainStormOptimalization::BrainStormOptimalization(CVRPInstance instance, int num_vehicles, IO_handlerV2::IO_handler io_handlers_v2, brainConfig config) : instance(instance), io_handlers_v2(io_handlers_v2), num_vehicles(num_vehicles), config(config)
{
    num_of_customers = instance.nodes.size() - 1;
    L = static_cast<int>(0.25 * num_vehicles) + 1; // L = [0.25R]+1

    if (L < 2)
    {
        L = 2;
    }
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
      perform_regert_cost_insertion(routes,instance.nodes);


    //DEBUGGIN ONLY
    /*
    double total_cost = 0.0;
    std::cout << "\n\t\t================= ETAP PIERWSZY - regret cost insertion, ";
    if (check_costs(routes, -1))
    {
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
     
    }
    total_cost = 0;
    for (int roz = 0; roz < routes.size(); ++roz)
    {
        total_cost += routes[roz].route_cost;
    }
    std::cout << " Calkowity KOSZT : " << total_cost << " =================";
    */
    //END FOR TESTING


    //Local serach
    local_search(routes);
    /*while (true)
    {
        while (perform_first_improvement_2_opt(routes)) //1
        {
            //Stosuj 2-opt tak dlugo az przynosi to poprawe       
                           
        }
        if (perform_first_improvement_exchange(routes)) //2
        {
            continue; //jest poprawa wroc do 2-opt
        }
        if (perform_first_improvement_cross(routes)) //3 
        {       
            
            continue;
        }
      
        if (perform_first_improvement_relocation(routes)) //4 - zwraca zly koszt - naprawione - przypadek single route
        {
            continue;
        }
        break;
    }
    */

    //FOR TESTING
    /*
    std::cout << "\n\t\t================= ETAP DRUGI - Local serach, ";
    if (check_costs(routes, -2))
    {
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

    }
    total_cost = 0;
    for (int roz = 0; roz < routes.size(); ++roz)
    {
        total_cost += routes[roz].route_cost;
    }
    std::cout << " Calkowity KOSZT : " << total_cost << " =================";,
    */
    //END FOR TESTING
 

    //Single route improvemnt - szukaj poprawy dla kazdej trasy z osobna
    for (int i = 0; i < routes.size(); ++i)
    {
        single_route_improvement(routes[i], config.T1, config.single_route_improvement_margin);
    }

    //FOR TESTING
    /*
    std::cout << "\n\t\t================= ETAP TRZCI - Single route improvement, ";
    if (check_costs(routes, -2))
    {
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

    }
    total_cost = 0;
    for (int roz = 0; roz < routes.size(); ++roz)
    {
        total_cost += routes[roz].route_cost;
    }
    std::cout << " Calkowity KOSZT : " << total_cost << " =================";
    */
    //END FOR TESTING
    
    
   
   
    return routes;
}


void BrainStormOptimalization::run()
{
    //OZNACZNIA
    //sb - najlepsze rozwiazanie 
    //spb - perturbed sb
    //s - rozwiazanie uzyskane 

    //TODO
    std::cout << "\n\t\t\t=========BRAIN STORM OPTIMALIZATION STARTED=========\n";
    std::vector<Route> sb = construct_initial_solution(); //3.1 Initialization of the best-so-far solution
  
    double sb_cost = get_sum_of_route_cost(sb); // koszt najlepszego rozwiazania
    double s_cost = 0.0; // koszt rozwiazanie uzyskanego po polaczeniu podproblemow
    //NIE JEST TO ZROBIONE NA GOTOWO - TYLKO TAK NA SZYBKO ABY BYL ZARYS GDZIE I JAK Z CZEGO KORZYSTAC

    bool stopping_condition = false;
    int my_stopping_condition = 3;
    int iterations = 0;
    //2: while the stopping condition is not satisfied do
    while (iterations < config.main_loop_itarations) //SZKIELET GLOWNEJ PETLI LINA 2
    {
        //funkcja nie modyfikuje wejscia (routes) tylko zwraca zmodyfikowany element
        std::vector<Route> spb = perturbation(sb, config.alfa_1);

        std::vector<std::vector<Route>> sub_problems = decomposition(spb);
        for (int i = 0; i < sub_problems.size(); ++i) // linia 8
        {
            divergent_operation(sub_problems[i]);   //linia 9 - 13 // po tym kroku sub_problem ma tylko poprawione rozwiazanie, nie zwraca nic gorszego nic wejscie
        }

        //skladanie rozwiazania
        s_cost = 0.0;
        for (int i = 0; i < sub_problems.size(); ++i) // linia 8
        {
            s_cost += get_sum_of_route_cost(sub_problems[i]); // koszt calego rozwiazania po polaczeniu podproblemow
        }

        if (s_cost < sb_cost)
        {
            sb_cost = s_cost;
            sb.clear();

           
            size_t total_routes = 0;
            for (const auto& sub : sub_problems) total_routes += sub.size();
            sb.reserve(total_routes);

            
            for (const auto& sub : sub_problems)
            {
                sb.insert(sb.end(), sub.begin(), sub.end());
            }

            std::sort(sb.begin(), sb.end(), [](const Route& a, const Route& b) {
                return a.vehicle_id < b.vehicle_id;
                });
        }
        iterations++;
    }



    //std::cout << "\n\t\t================= OSTATNI ETAP =================";

    double total_cost = 0;
    if (check_costs(sb, -2))
    {
        for (int roz = 0; roz < sb.size(); ++roz)
        {
            std::cout << "\n#" << roz << " Koszt z route_cost : " << sb[roz].route_cost;
            double my_cost = 0.0;
            for (int i = 0; i < sb[roz].customers.size(); ++i)
            {
                my_cost += sb[roz].arrival_times[i];

            }
            double przeliczone = g(sb[roz]);

            std::cout << "\n#" << roz << " Koszt z arrival_times : " << my_cost;
            std::cout << "\n#" << roz << " Przeliczony  : " << przeliczone;
            total_cost += my_cost;
        }

    }
    total_cost = 0;
    for (int roz = 0; roz < sb.size(); ++roz)
    {
        total_cost += sb[roz].route_cost;
    }
    std::cout << " Calkowity KOSZT : " << total_cost << " =================";
    result.total_cost = total_cost;
    result.routes = sb;

}

const Result& BrainStormOptimalization::get_result() const {
    return result;
}


std::vector<Route> BrainStormOptimalization::perturbation(std::vector<Route>& sb, double alfa)
{
    std::vector<Route> spb = sb;
    double total_cost = 0.0;


    double epsilon = random_01();
    int number_of_moving_customers = static_cast<int>(floor(alfa * num_of_customers + 5 * epsilon)); // moc zbioru r1

    //losowanie klientow 
    std::vector<std::pair<int, int>> selected = get_n_random_clients(spb, number_of_moving_customers);

    std::sort(selected.begin(), selected.end(),
        [](const std::pair<int, int>& a, const std::pair<int, int>& b)
        {
            if (a.first != b.first)
                return a.first < b.first;      // rosonoco po trasie
            return a.second > b.second;        // malejaco po kliencie
        });

    //usuniecie wylosowanych klientow i dodanie ich do zbioru r1
    std::vector<Node> r1;
    double new_cost_r1 = 0.0;
    std::vector<double> next_times1;
    for (int i = 0; i < selected.size(); i++)
    {
        
        Route& selected_route = spb[selected[i].first];
        int customer_index = selected[i].second;
       
        //Wyznaczenie kosztu trasy po usunieciu klienta
        new_cost_r1 = calculate_removal_cost_for_relocation(selected_route, customer_index, next_times1);

        //zapisz klienta
        Node client_to_remove = selected_route.customers[customer_index];
        r1.push_back(client_to_remove);

        selected_route.remaining_capacity += client_to_remove.demand; 
        selected_route.customers.erase(selected_route.customers.begin() + customer_index); 
        selected_route.route_cost = new_cost_r1; 
        selected_route.arrival_times = next_times1; 
    }


   
    perform_regert_cost_insertion(spb, r1); // przeprowadzenie procedury dla kazdego klienta
    return spb;
}



double BrainStormOptimalization::compute_T(const Route& R1, const Route& R2)
{
    if (R1.customers.empty() || R2.customers.empty())
        return 1e9; 

    

    int N1 = R1.customers.size() - 1;
    int N2 = R2.customers.size() - 1;

    double sum1 = 0.0;
    for (int i = 1; i < R1.customers.size(); ++i)
    {
        const Node& u = R1.customers[i];
        double min_dist = 1e9;
        for (int j = 1; j < R2.customers.size(); ++j)
        {
            min_dist = std::min(min_dist, euclidean_distance(u, R2.customers[j]));  // R2.customers[j]) -> v 
        }
        sum1 += min_dist;
    }


    double sum2 = 0.0;
    for (int i = 1; i < R2.customers.size(); ++i)
    {
        const Node& u = R2.customers[i];
        double min_dist = 1e9;
        for (int j = 1; j < R1.customers.size(); ++j)
        {
            min_dist = std::min(min_dist, euclidean_distance(u, R1.customers[j]));  // R1.customers[j]) -> v 
        }
        sum2 += min_dist;
    }
   
    return (sum1 / N1) + (sum2 / N2);
}

//DZIELE NA ROWNE GRUPY PO X ELEMENTOW, TYLKO OSTATNIA GRUPA MOZE MIEC X + X-1 ELEMNTOW, CZYLI GRUPA NIE MOZE BYC MNIEJSZA OD X 
//w praktyce chyba zawsze beda trzy grupy, jezeli chcialbym zachowac ze grupy sa miedzy L a 3/2L to wtedy 
//wiecej grup musialo byc miec rozne rozmiary, a nie tylko ostatnia grupa tak jak jest to opisane
std::vector<std::vector<Route>> BrainStormOptimalization::decomposition(std::vector<Route>& routes)
{
    std::vector<std::vector<Route>> partial_solutions;
    std::vector<Route> subproblem;
    int v1_index = random_int_from_to(0, routes.size() - 1);
    std::vector<std::pair<int,double>> indicator_values;
    
 
    double tree_two_L = (3.0 * L) / 2.0;
    if (tree_two_L >= routes.size()) // przypadek gdy da sie zrobic tylko jedna grupe, lub tylko dwie przy czy ta druga bedzie za mala
    {
         partial_solutions.push_back(routes); //zwracamy orginal
         return partial_solutions;
    }


    for (int i = 0; i < routes.size(); ++i)
    {
        if (i == v1_index)
        {
            continue;
        }
        else
        {
            indicator_values.emplace_back(i, compute_T(routes[v1_index], routes[i]));
        }      
    }

    std::sort(indicator_values.begin(), indicator_values.end(),
        [](const std::pair<int, double>& a, const std::pair<int, double>& b)
        {
            return a.second < b.second;
        });

    //v1 pierwsza trasa pierwszego podproblemu
    subproblem.push_back(routes[v1_index]);
   

    for (int i = 0; i < L - 1; ++i) //  L-1 pierwszych tras trafia do podproblemu z v1
    {
        subproblem.push_back(routes[indicator_values[i].first]);
        indicator_values[i].second = -1.0; // oznaczenie ze nie moge wykorzystac juz tego elemenmtu
    }
    
    partial_solutions.push_back(subproblem); // [v1,.. L-1]

   
    //to juz w petli dodajemy kolejene wartosci
    int remaining_routes = indicator_values.size() - L + 1;
   
    int counter = 1;
 //   while (remaining_routes >= tree_two_L)
     while (remaining_routes >= L)
    {
       subproblem.clear();
       int index = indicator_values[counter * L - 1].first;

       subproblem.push_back(routes[index]);  //v2, v3, ...

       for (int i = counter * L; i < indicator_values.size(); ++i)
       {
           indicator_values[i].second = compute_T(routes[index], routes[i]);
       }
       std::sort(indicator_values.begin() + counter * L, indicator_values.end(),
           [](const std::pair<int, double>& a, const std::pair<int, double>& b)
           {
               return a.second < b.second;
           });
       int start_index = counter * L;
       for (int i = start_index; i <= start_index + L - 2  ; ++i) // L-1 KOLEJNYCH tras trafia do podproblemu
       {
           subproblem.push_back(routes[indicator_values[i].first]);
           indicator_values[i].second = -1.0; // oznaczenie ze nie moge wykorzystac juz tego elemenmtu
       }
       partial_solutions.push_back(subproblem);
       remaining_routes -= L;
       counter++;
    }

    //nie trzeba nic przeliczac po porstu wrzuc pozostale trasy
    // PIERWSZE ROZWIAZANIE, ALE TWORZYLO TO ZA MALE OSTATNIE GRUPY
    //if (counter * L - 1 < routes.size())
    //{      
    //    subproblem.clear();
    //    int start_index = counter * L - 1;
    //    for (int i = start_index; i < indicator_values.size(); ++i) //  dodanie wszystkich poozostalych tras 
    //    {
    //        subproblem.push_back(routes[indicator_values[i].first]);
    //        indicator_values[i].second = -1.0;
    //    }
    //    partial_solutions.push_back(subproblem);
    //}
  
    if (counter * L - 1 < routes.size())
    {      
        std::vector<Route>& last_subproblem = partial_solutions.back();
        int start_index = counter * L - 1;
        for (int i = start_index; i < indicator_values.size(); ++i) //  dodanie wszystkich poozostalych tras 
        {
            last_subproblem.push_back(routes[indicator_values[i].first]);
            indicator_values[i].second = -1.0;
        }
       
    }
    return partial_solutions;
}


bool BrainStormOptimalization::divergent_operation(std::vector<Route>& spb)
{
    bool improved = false;
    double best_cost = 0.0;
    for (int i = 0; i < spb.size(); ++i)
    {
        best_cost += spb[i].route_cost;
    }
    std::vector<Route> my_solution = spb;
    std::vector<Route> my_best_solution = spb;
    for (int i = 1; i <= config.N; ++i) //linia 9
    {
       
        int random_operator = random_int_from_to(1, 3);

        switch (random_operator)
        {
            case 1:
            {
                std::vector<std::pair<int, int>> random_clients;
                int i = 1;
                while (i <= config.T2)
                {
                    random_clients = get_n_random_clients_diff_routes(my_solution, 2);
                    Move move = { random_clients[0].first, random_clients[0].second,random_clients[1].first, random_clients[1].second };
                    int res = first_operator(my_solution, move); // w my_solution jest albo orginal, albo poprawiona trasa 
                    if (res != -1)
                    {
                        i++; // zalicz probe jezeli udalo sie wykonac ruch
                    }
                }
                break;
            }
            case 2:
            {
                std::vector<std::pair<int, int>> random_clients;
                int i = 1;
                while (i <= config.T2)
                {
                    random_clients = get_n_random_clients_diff_routes(my_solution, 2, 2, true); // nie mozna wylosowac pierwszego oraz ostatniego elemntu
                    Move move = { random_clients[0].first, random_clients[0].second,random_clients[1].first, random_clients[1].second };
                    int res = second_operator(my_solution, move);
                    if (res != -1)
                    {
                        i++; // zalicz probe jezeli udalo sie wykonac ruch
                    }
                }
                break;
            }

            case 3:
            {
                
                my_solution = perturbation(my_solution, config.alfa_2);                   
                break;
            }
        }
        //linia 11
        local_search(my_solution);
        for (int r = 0; r < my_solution.size(); ++r)
        {
            
            single_route_improvement(my_solution[r], config.T1);
        }   


        double new_cost = 0.0;
        for (int i = 0; i < my_solution.size(); ++i)
        {
            new_cost += my_solution[i].route_cost;
        }
        if (new_cost < best_cost) // jezeli jest poprawa to zapisz
        {
            improved = true; // jezeli udalo sie poprawic chociaz jeden raz to zwroc wtedy true
            best_cost = new_cost;
            
            spb = my_solution;
        }
        else
        {
            my_solution = spb;
        }
    }
    return improved;
}



void BrainStormOptimalization::local_search(std::vector<Route>& routes)
{
    while (true)
    {
        while (perform_first_improvement_2_opt(routes)) //1
        {
            //Stosuj 2-opt tak dlugo az przynosi to poprawe       

        }
        if (perform_first_improvement_exchange(routes)) //2
        {
            continue; //jest poprawa wroc do 2-opt
        }
        if (perform_first_improvement_cross(routes)) //3 
        {

            continue;
        }

        if (perform_first_improvement_relocation(routes)) //4 - zwraca zly koszt - naprawione - przypadek single route
        {
            continue;
        }
        break;
    }
}