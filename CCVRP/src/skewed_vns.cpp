#include "skewed_vns.h"



    Skewed_VNS::Skewed_VNS(CVRPInstance instance, int num_vehicles, IO_handlerV2::IO_handler io_handlers_v2, SkewedVNSConfig config): instance(instance),io_handlers_v2(io_handlers_v2), num_vehicles(num_vehicles), config(config)
    {
      
    }

    void Skewed_VNS::run()
    {

        std::vector<Route> routes = constructive_heurestic();
        
        for (int i = 0; i < routes.size(); i++)
        {
            routes[i].route_cost = g(routes[i]); // oblicz koszt trasy
        }
        cost_progress.push_back(calculate_cost(routes));    // ZAPIS

     
        result.routes = SVNS(routes);
		//


        double result_cost = 0.0;
        for (int i = 0; i < result.routes.size(); i++)
        {
            result_cost += result.routes[i].route_cost;
        }

        result.total_cost = result_cost;
        result.cost_progress = this->cost_progress;
    }

    const Result& Skewed_VNS::get_result() const {
        return result;
    }

    std::vector<Route> Skewed_VNS::constructive_heurestic()
    {
        std::vector<DistanceInfo> distances_from_depot = get_all_distances(instance.depot_id - 1, instance);
        std::sort(distances_from_depot.begin(), distances_from_depot.end());
        std::vector<Route> routes;
        for (int i = 0; i < num_vehicles; ++i) {
            routes.push_back(Route(i, instance.capacity));  // Pojazd i o odpowiedniej pojemnoœci
        }

        //dodajemy po kolei najepsze trasy do kolejnych pojazdow
        std::vector <int> indexes_to_remove;
        for (auto& c : routes)
        {
            auto best = distances_from_depot.front();
            distances_from_depot.erase(distances_from_depot.begin());
            //best second to indeks w oryginalnej tablicy instances z wejœciowymi danymi
            //c.add_customer(instance.nodes[best.second], best.first, 1);
            
            //szukanie indeksu klienta na podstawie node_id
            auto it = std::find_if(instance.nodes.begin(), instance.nodes.end(),
                [&](const Node& n) { return n.id == best.node_id; });
            if (it != instance.nodes.end()) {
                c.add_customer_at_index(*it, 1, best.distance);           
                instance.nodes.erase(it);
            }
            else {
                std::cerr << "Nie znaleziono klienta o ID " << best.node_id << "!\n";
            } 
            //indexes_to_remove.push_back(best.node_id);
           
        }
        //usuwanie dodanych klientow z listy wszstkich klientów
        //std::sort(indexes_to_remove.begin(), indexes_to_remove.end(), std::greater<int>());
        //for (int index : indexes_to_remove) {
        //    if (index >= 0 && index < instance.nodes.size()) {
        //        instance.nodes.erase(instance.nodes.begin() + index);
        //    }
        //}
        //zwolnie pamiêci
        //std::vector<int>().swap(indexes_to_remove);
       
        //wstawienie pozostalych klientow
        //usuniecie magazynu z listy klientów
        instance.nodes.erase(instance.nodes.begin());
        //Przejœcie przez wszystkich klientow
        for (int i = instance.nodes.size() - 1; i >= 0; i--)
        {
         
            //sprawdzanie kosztu wstawienia wszsytkich klientow i
            InsertionResult best_feasible_insertion;
            InsertionResult best_any_insertion;
            best_feasible_insertion.cost = std::numeric_limits<double>::infinity();
            best_any_insertion.cost = std::numeric_limits<double>::infinity();

			//przejscie przez wszysktie pojazdy - szukanie najlepszej trasy dla klienta i
            for (int r = 0; r < routes.size(); r++)
            {
                //przekazanie trasy r oraz kliena i - sprawdzenie kosztu wstawienie we wszystkich trasach
                InsertionResult insertion = find_best_insertion(routes[r], instance.nodes[i]);
                // Zapisz najlepsz¹ wstawkê tylko jeœli trasa ma wolne miejsce
                if (instance.nodes[i].demand <= routes[r].remaining_capacity)
                {
                    if (insertion.cost < best_feasible_insertion.cost)
                    {
                        best_feasible_insertion = insertion;
                    }
                }
                // Niezale¿nie od pojemnoœci – zapisz najlepsz¹ ogólnie
                if (insertion.cost < best_any_insertion.cost)
                {
                    best_any_insertion = insertion;
                }
               // std::cout << "Najlepszy wynik dla trasy: " << r <<" -  " << insertion.cost << std::endl;
            }
            InsertionResult chosen_insertion = (best_feasible_insertion.cost < std::numeric_limits<double>::infinity())
                ? best_feasible_insertion
                : best_any_insertion;
          //std::cout << "  Najlepszy koszt:  " << best_any_insertion.cost <<"  Bez przekraczania: " << best_feasible_insertion.cost << std::endl;
            //routes[chosen_insertion.route_id].add_customer(instance.nodes[i], chosen_insertion.cost, chosen_insertion.place, false);
            
			//std::cout <<"  Klient: " << i << " trafia na pozycje: " << chosen_insertion.place << " do pojazdu: " << chosen_insertion.route_id << std::endl;
            routes[chosen_insertion.route_id].add_customer_at_index(instance.nodes[i], chosen_insertion.place + 1 ,chosen_insertion.cost);
            instance.nodes.erase(instance.nodes.begin() + i);
        }
        return routes;
    }

    double Skewed_VNS::f(std::vector<Route>& s, const std::vector<int>& changed_routes, bool test)
    {

        double result = 0.0;
        double capacity_penalty = 0.0;
        int m = s.size(); // number of routes/ cars

        //przeliczanie wszystkich tras
        if (changed_routes.empty())
        {
            for (int k = 0; k < m; k++)
            {
                //double cost = g(s[k]);
				//s[k].route_cost = cost;
                result += g(s[k]); //max(0, s[k].initial capacity - s[k].ile ma zaladowane = ile zostalo miejsca)
                capacity_penalty += std::max(0, s[k].remaining_capacity);
 
            }
        }

		//przeliczanie tylko zmienionych tras - reszta korzysta z pola route_cost
        else
        {
            for (int k : changed_routes)
            {            
				//s[k].route_cost = g(s[k]);
                g(s[k]);
            }
            for (const auto& route : s)
            {
                result += route.route_cost;
                capacity_penalty += std::max(0, route.remaining_capacity);        
            }

			// DO TESTOWANIA CZY KOSZTY SIE ZGADZAJA
            if (test)
            {
            
            int idx = 0;
            for (auto& route : s)
                {
                    double saved_cost = route.route_cost;
                    double calculated_cost = g(route);
                    if (saved_cost != calculated_cost) {
                        std::cout << "UWAGA: route " << idx++
                            << " cost mismatch: saved=" << saved_cost
                            << " recalculated=" << calculated_cost << "\n";
                    }
                
                }
            }

        }
        
        capacity_penalty *= config.f_alfa;
        result += capacity_penalty; // penelty for capacity violations

    
        return result;
    }

    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    ///Pierwotne obliczanie delty - na podstawie liczby tras
    //double Skewed_VNS::delta(std::vector<Route>& s1, std::vector<Route>& s2)
    //{
    //    int n = s1.size() - 1; //tyle sprawdzamy ile jest nastepcow 
    //    return 1.0 + (config.delta_alfa * (static_cast<double>(dif(s1, s2)) / n));
    //}


    //Rozwiazanie na podstawie liczby klientow
    double Skewed_VNS::delta(std::vector<Route>& s1, std::vector<Route>& s2)
    {
        // liczba wszystkich klientów (bez depot)
        int n = 0;
        for (const auto& route : s1)
        {
            n += route.customers.size() - 1;
        }
		
        if (n == 0) {
           
            return 1.0;
        }

        return 1.0 + (config.delta_alfa * static_cast<double>(dif(s1, s2)) / n);
    }

    std::vector<Route> Skewed_VNS::SVNS(std::vector<Route>& solution, int k_max)
    {
		//tylko do wypisywania postepow
        double last_best_total_cost = 0;
        
		calculate_remaining_capacity(solution);

        float beta = 1.02;
       
        int k;
        std::vector<Route> s = solution; // operujemy na kopi
        std::vector<Route> s_best = s; // == s* w opisie algorytmu

        std::vector<Route> s_prim;
        std::vector<Route> s_bis;
        std::vector<Route> s_test;

        //Main loop end condition 
        int no_improve_count = 0;
        
        int max_no_improve = config.SVNS_max_no_improve;
        while (no_improve_count < max_no_improve)
        {

            k = 1;
            while (k <= k_max)
            {
                s_prim = s;
 
                //?????
                calculate_remaining_capacity(s);
				//TAK BYLO NA POCZATKU
               /* for (int i = 1; i <= k; i++)
                {
                    s_prim = P(s_prim, k); //perturbation
                }
                */
           
              
                //double przed_P = calculate_cost(s_prim); //to bylo

             
				s_test = P(s_prim, k); //perturbation
				s_prim = s_test;
				double po_P = calculate_cost(s_prim);

                calculate_remaining_capacity(s_prim);
                s_bis = VND(s_prim);        //local search 
				double po_VND = calculate_cost(s_bis);

                double f_s = f(s);
                double f_s_bis = f(s_bis);
                double delta_s_s_bis = delta(s, s_bis);
            
                double f_s_best = f(s_best);
                if (f_s_bis < std::min(f_s * delta_s_s_bis, beta * f_s_best))  //skewed move 
                {
                    s = s_bis;
                    k = 1;
                }
                else
                {             
                    k = k + 1;
                }

                Result s_best_result;
                if (f_s_bis < f_s_best)
                {
                    s_best = s_bis;
                    no_improve_count = 0;
                    //tymczasowe !!!
                    //TU BYLO RESULT_S_BEST_TESULT - TO KTORE JEST WYCIAGNIECTE POZA TEGO IFA
					s_best_result.routes = s_best;                                
                    for (int i = 0; i < s_best_result.routes.size(); i++)
                    {
                        s_best_result.total_cost += s_best_result.routes[i].route_cost;
                    }
					
                  


                }
                else
                {
                    no_improve_count++;
                }

				//wypisywanie postepow
                if (s_best_result.total_cost != 0)
                {
                    last_best_total_cost = s_best_result.total_cost;
                }
                cost_progress.push_back(last_best_total_cost); // ZAPIS
                //std::cout << "\r" << std::string(200, ' ') << "\r"
                //    << "No improve iterations: " << no_improve_count
                //    << "          Current best total cost: "
				//	<< last_best_total_cost << "    k = " << k
				//	<< " Przed P: " << przed_P << " Po P: " << po_P << " Po VND: " << po_VND
                //    << std::flush;
              
            }
        }
        return s_best;
    }

    std::vector<Route> Skewed_VNS::N(std::vector<Route>& s, int neighborhood)
    {
        double f_s = f(s);
        std::vector<Route> s_prime = s;
   
		


        switch (neighborhood) {
        case 1:
        { //iteracja przez wyszstkie pojazdy
            for (int r = 0; r < s.size(); r++)
            {
                //iteracja przez wszystkich klientow - intra route - nie wychodze poza dana trase, poza baza, ona nie moze byc przestawiona
                //Zminaa c z 1 na 0

				std::vector<int> changed_routes = {r };

                int route_size = s_prime[r].customers.size();
                for (int c = 1; c < route_size; c++)
                {
                    auto client = s_prime[r].customers[c]; // klient ktorego przestawiam
                    s_prime[r].customers.erase(s_prime[r].customers.begin() + c);

                    for (int i = 1; i < route_size; i++) // miejsce za ktore wstawic klienta c, czyli np i = 2 to c bedzie na pozycji 3 
                    {
                        if (c == i) continue; // nie przestawiamy klienta na jego aktualna pozycje
                        s_prime[r].customers.insert(s_prime[r].customers.begin() + i, client); // wstawiam klienta na miejsce i
                        if (f_s > f(s_prime, changed_routes))
                        {          
                            return s_prime; // zwracam pierwsze lepsze s_prime
                        }
						// brak poprawy - cofniece proby
                        s_prime[r].customers.erase(s_prime[r].customers.begin() + i);
                        
                    }
					//powrot do oryginalnej pozycji klienta
                    s_prime[r].customers.insert(s_prime[r].customers.begin() + c, client);
					//ponowne przeliczenie kosztu trasy po przywrocenie jej stanu
                    g(s_prime[r]);
                }
            }

            //inter szukanie poprawy poprzez zamiane miedzy roznymi pojazdami
            for (int r = 0; r < s.size(); r++)
            {
                for (int rr = 0; rr < s.size(); rr++)
                {
                   
                    if (r == rr) continue;
                    std::vector<int> changed_routes = {rr,r};
                    //iteracja przez kolejnych klientow w trasie r
                    for (int i = 1; i < s[r].customers.size(); i++)
                    {
                        auto client = s_prime[r].customers[i]; // klient ktorego przestawiam
                        s_prime[r].customers.erase(s_prime[r].customers.begin() + i); // usuwam klienta ktorego przestawiam
                        s_prime[r].remaining_capacity += client.demand; // przywracamy pojemnosc pojazdu, bo klient zostal usuniety
                        for (int ii = 1; ii <= s[rr].customers.size(); ii++)
                        {
                            s_prime[rr].customers.insert(s_prime[rr].customers.begin() + ii, client); // wstawiam klienta do innego pojazdu na miejsce ii
                            s_prime[rr].remaining_capacity -= client.demand; // przywracamy pojemnosc pojazdu, bo klient zostal usuniety
                            if (f_s > f(s_prime, changed_routes))
                            {
                                
                                return s_prime; // zwracam pierwsze lepsze s_prime
                            }
							// brak poprawy - cofniece proby
                                s_prime[rr].remaining_capacity += client.demand; // przywracamy pojemnosc pojazdu, bo klient zostal usuniety
                               // s_prime[r].remaining_capacity -= client.demand; // przywracamy pojemnosc pojazdu, bo klient zostal usuniety
                                s_prime[rr].customers.erase(s_prime[rr].customers.begin() + ii); // usuwam wstawionego klienta z innego pojazdu
                               // s_prime[r].customers.insert(s_prime[r].customers.begin() + i, client); //wstawiam klienta z powrotem do oryginalnego pojazdu 
                        }
                        s_prime[r].customers.insert(s_prime[r].customers.begin() + i, client);
                        s_prime[r].remaining_capacity -= client.demand;
                        
					

                        g(s_prime[rr]);
                    }
                    g(s_prime[r]);
                }
            }
            return s_prime;
            break;
        }
        case 2:
        {
            int block_size = config.block_size; // rozmiar bloku do przestawienia
            //iteracja przez wyszstkie pojazdy
            std::vector<Route> s_tmp = s;
            for (int r = 0; r < s.size(); r++)
            {
                //iteracja przez wszystkich klientow - intra route - nie wychodze poza dana trase, poza baza, ona nie moze byc przestawiona
                std::vector<int> changed_routes = {r};

                if (s[r].customers.size() <= config.block_size)
                {
					block_size = s[r].customers.size() > 1 ? s[r].customers.size() - 1 : 0;
                }
                
                for (int c = 1; c <= s[r].customers.size() - block_size; c++)
                {
                    std::vector<Node> block(s_prime[r].customers.begin() + c, s_prime[r].customers.begin() + c + block_size);
                    s_prime[r].customers.erase(s_prime[r].customers.begin() + c, s_prime[r].customers.begin() + c + block_size);
                    
                    for (int i = 1; i <= s[r].customers.size() - block_size; i++)
                    {
                        if (c == i) continue;
                      
                        // int insertPos = (i > c) ? i - block_size : i;
                        s_prime[r].customers.insert(s_prime[r].customers.begin() + i, block.begin(), block.end());

                        if (f_s > f(s_prime, changed_routes))
                        {                  
                            return s_prime; // zwracam pierwsze lepsze s_prime
                        }
                            // nie ma poprawy to wrc do oryginalnego rozwiazania
                            s_prime[r].customers.erase(s_prime[r].customers.begin() + i, s_prime[r].customers.begin() + i + block_size);
                           // s_prime[r].customers.insert(s_prime[r].customers.begin() + c, block.begin(), block.end());
                           // s_prime = s_tmp;
                    }
                    s_prime[r].customers.insert(s_prime[r].customers.begin() + c, block.begin(), block.end());
                    g(s_prime[r]);
                }
            }

            //inter szukanie poprawy poprzez zamiane miedzy roznymi pojazdami
            for (int r = 0; r < s.size(); r++)
            {
                for (int rr = 0; rr < s.size(); rr++)
                {
                    if (r == rr) continue;
                    std::vector<int> changed_routes = { rr, r };
                    if (s[r].customers.size() <= config.block_size)
                    {
                        block_size = s[r].customers.size() > 1 ? s[r].customers.size() - 1 : 0;
                    }
                    //iteracja przez kolejnych klientow w trasie r
					
                    for (int c = 1; c <= s[r].customers.size() - block_size; c++)
                    {
                        std::vector<Node> block(s_prime[r].customers.begin() + c, s_prime[r].customers.begin() + c + block_size);
                        double block_demand = 0;
                        for (const auto& client : block) {
                            block_demand += client.demand;
                        }

                        s_prime[r].customers.erase(s_prime[r].customers.begin() + c, s_prime[r].customers.begin() + c + block_size);
                        s_prime[r].remaining_capacity += block_demand;

                        for (int i = 1; i <= s[rr].customers.size(); i++)
                        {

                            s_prime[rr].customers.insert(s_prime[rr].customers.begin() + i, block.begin(), block.end());
                            s_prime[rr].remaining_capacity -= block_demand;       
                            if (f_s > f(s_prime, changed_routes))
                            {
                              
                                return s_prime; // zwracam pierwsze lepsze s_prime
                            }
                            // nie ma poprawy to wrc do oryginalnego rozwiazania
                            
                                //TUUU
                               // s_prime = s_tmp;
                                s_prime[rr].customers.erase(s_prime[rr].customers.begin() + i, s_prime[rr].customers.begin() + i + block_size);
                                s_prime[rr].remaining_capacity += block_demand;
                              //
                              //  s_prime[r].customers.insert(s_prime[r].customers.begin() + c, block.begin(), block.end());
                              //  s_prime[r].remaining_capacity -= block_demand;   
                        }
                        s_prime[r].customers.insert(s_prime[r].customers.begin() + c, block.begin(), block.end());
                        s_prime[r].remaining_capacity -= block_demand;
                       
                        
                    }
                    g(s_prime[rr]);
                }
                g(s_prime[r]);
            }         
            return s_prime;
            break;
        }
        case 3:
        {
            //iteracja przez wyszstkie pojazdy
            for (int r = 0; r < s.size(); r++)
            {
                std::vector<int> changed_routes = { r };
                //iteracja przez wszystkich klientow - intra route - nie wychodze poza dana trase, poza baza, ona nie moze byc przestawiona
                for (int c = 1; c < s[r].customers.size() - 1; c++)
                {
                    for (int i = c + 1; i < s[r].customers.size(); i++)
                    {
                        std::swap(s_prime[r].customers[c], s_prime[r].customers[i]);
                        if (f_s > f(s_prime, changed_routes))
                        {  
                            return s_prime; // zwracam pierwsze lepsze s_prime
                        }
                        else // nie ma poprawy to wrc do oryginalnego rozwiazania
                        {
                            std::swap(s_prime[r].customers[c], s_prime[r].customers[i]);
                        }
                    }
                }
                g(s_prime[r]);
            }
            //inter szukanie poprawy poprzez zamiane miedzy roznymi pojazdami
            for (int r = 0; r < s.size() - 1; r++)
            {
                for (int rr = r + 1; rr < s.size(); rr++)
                {
                    std::vector<int> changed_routes = { rr, r };
                    //iteracja przez kolejnych klientow w trasie r
                    for (int i = 1; i < s[r].customers.size(); i++)
                    {
                        for (int ii = 1; ii < s[rr].customers.size(); ii++)
                        {
                            Node client_r = s_prime[r].customers[i];
                            Node client_rr = s_prime[rr].customers[ii];

                            std::swap(s_prime[r].customers[i], s_prime[rr].customers[ii]);
                            s_prime[r].remaining_capacity += client_r.demand;
                            s_prime[r].remaining_capacity -= client_rr.demand;

                            s_prime[rr].remaining_capacity += client_rr.demand;
                            s_prime[rr].remaining_capacity -= client_r.demand;

                            if (f_s > f(s_prime, changed_routes))
                            {
                        
                                return s_prime; // zwracam pierwsze lepsze s_prime
                            }
                            else // nie ma poprawy to wrc do oryginalnego rozwiazania
                            {
                                std::swap(s_prime[r].customers[i], s_prime[rr].customers[ii]);

                                s_prime[r].remaining_capacity += client_rr.demand;
                                s_prime[r].remaining_capacity -= client_r.demand;

                                s_prime[rr].remaining_capacity += client_r.demand;
                                s_prime[rr].remaining_capacity -= client_rr.demand;

                            }
                        }                  
                    }
                    g(s_prime[rr]);
                }
                g(s_prime[r]);
            }
            
            return s_prime;
            break;
        }
        default:
            break;
        }
    }

    //poprawione p1 i p2
    std::vector<Route> Skewed_VNS::P(std::vector<Route>& s, int procedure)
    {
        std::vector<Route> s_prime = s;
        std::vector<Route> s_best = s;
       // double f_s = f(s); // koszt oryginalnego rozwiazaia

       

        switch (procedure)
        {

            //P1 - przestawianie w obrebie jednej trasy klienta na wszystkie mozliwe miejsca w tej trasie i wybranie najelpeszego
        case 1:
        {
            for (int max_attempts = 0; max_attempts < 400; max_attempts++)
            {
                std::vector<double> next_times2;
                std::vector<double> best_times2;
                std::pair<int, int> random_client = get_random_client(s);
                auto& route = s_prime[random_client.first];

                if (route.customers.size() <= 2) continue;
                int k = random_client.second;
                Node client = route.customers[k]; // wylosowany klient dla ktorego sprawdzam wszystkie pozycje w jego trasie


                //1. Koszt po usunieciu klienta
                double reduction = 0.0;
                int n_after_client = route.customers.size() - 1 - k;
                if (k < route.customers.size() - 1) // klient nie byl ostatni
                {
                    double d_prev_k = euclidean_distance(route.customers[k - 1], route.customers[k]); // wyznacznie ustatnsu a -> k
                    double d_k_next = euclidean_distance(route.customers[k], route.customers[k + 1]); // k->b
                    double d_prev_next = euclidean_distance(route.customers[k - 1], route.customers[k + 1]); //a->b
                    reduction = (d_prev_k + d_k_next) - d_prev_next;
                }
                else // klient byl na koncu
                {
                    reduction = euclidean_distance(route.customers[k - 1], route.customers[k]);
                }

                double cost_without_k = route.route_cost - route.arrival_times[k] - (n_after_client * reduction); // koszt trasy bez klienta 

                double wejscie = route.route_cost;

                Route current_nodes = route;
                current_nodes.customers.erase(current_nodes.customers.begin() + k);

                double test_new_cost_without_k = g(current_nodes);


                double best_total_cost = std::numeric_limits<double>::max();
                int best_pos = -1;
                double new_cost_r2;
               
                for (int p = 1; p <= current_nodes.customers.size(); ++p) // sprawdzenie wszystkich pozycji
                {
                    if (p == random_client.second)
                    {
                        continue;
                    }
                    new_cost_r2 = calculate_insertion_cost_for_relocation(current_nodes, p, client, next_times2);


                    if (new_cost_r2 < best_total_cost) {
                      
                        best_total_cost = new_cost_r2;
                        best_pos = p;
                        best_times2 = next_times2;
                    }
                    
                }
                //zapisanie wyniku
                if (best_pos != -1 ) {

                    route.customers = current_nodes.customers;
                    route.customers.insert(route.customers.begin() + best_pos, client);
                    route.route_cost = best_total_cost;
                    route.arrival_times = best_times2;
                    return s_prime;
                }
                s_prime = s;  
            }
            return s;

        }

        //P2
        case 2:
        {
            int best_route=0, best_pos=0;
            double r1_cost=0, r2_cost=0;
            std::vector<double> best_times1;
            std::vector<double> best_times2;
            for (int max_attempts = 0; max_attempts < 20; max_attempts++)
            {
               
                //Szukamy najlepszej pozycja poza aktualna trasa
                std::pair<int, int> random_client = get_random_client(s);
                Route& r1 = s_prime[random_client.first];
              
                Node node_to_move = r1.customers[random_client.second]; // losowy klient ktorego przestawiam
                if (r1.customers.size() < 2) continue;

                double new_cost_r1, new_cost_r2;
                std::vector<double> next_times1, next_times2;
                //nowy koszt trasy po usunieciu z niej klienta
                new_cost_r1 = calculate_removal_cost_for_relocation(r1, random_client.second, next_times1);
   
                double best_delta = std::numeric_limits<double>::max();

                for (int r = 0; r < s.size(); r++)//przechodzimy przez kolejne trasy
                {

                    if (r == random_client.first)
                    {
                        continue; // nie przestawiamy klienta w tej samej trasie
                    }

                    Route& r2 = s_prime[r];
                    // Sprawdzenie pojemnoœci raz dla ca³ej trasy
                    if (node_to_move.demand > r2.remaining_capacity)
                        continue;


                    for (int c = 1; c <= r2.customers.size(); c++) // przejscie przez wszyskie pozycje w innej trasie
                    {
                        
                        new_cost_r2 = calculate_insertion_cost_for_relocation(r2, c, node_to_move, next_times2);

                        //sumaryczny koszt tras po zmianie
                        double new_sum_cost = new_cost_r1 + new_cost_r2;
                       

             
                        if (best_delta > new_sum_cost) // oznacza ze poprawa
                        {
                            //  std::cout << "P2 POPRAWA \n";
                            best_delta = new_sum_cost;
                            best_times1 = next_times1; // nowy arrival_times dla trasy z ktorej uzuwam
                            best_times2 = next_times2; // nowy arrival_times dla trasy do ktorej wstawiam
                            r1_cost = new_cost_r1;
                            r2_cost = new_cost_r2;
                            best_pos = c;
                            best_route = r;
                            
                        }
                    }
                }
                if (best_pos == 0)
                {
                    continue;
                }
                //ZAPISANIE WYNIKU
                r1.customers.erase(r1.customers.begin() + random_client.second); // usun klienta
                r1.remaining_capacity += node_to_move.demand;
                r1.arrival_times = best_times1;
                r1.route_cost = r1_cost;
                //std::cout << "\n pozzucja:" << best_pos << " rozmiar trasy: ";
                //std::cout << s_prime[best_route].customers.size() << "\n";
                s_prime[best_route].customers.insert(s_prime[best_route].customers.begin() + best_pos, node_to_move);
                s_prime[best_route].remaining_capacity -= node_to_move.demand;
                s_prime[best_route].arrival_times = best_times2;
                s_prime[best_route].route_cost = r2_cost;
                
                return s_prime; // zwraca najlepsze rozwiazanie - usuwa losowego klienta i wstawia go na najlepsza pozycje
            }
            return s; // w ogle nie udalo sie znalezc poprawy w max_attempts - zwroc orginal

        }
        case 3:
        { // P3 wymianiamy dwoch losowych klientow z losowych tras nastepnie przestawiamy klienta z pierwszej trasy na kazda inna pozycje w kazdej innej trasie poza trasa 
            std::pair<int, int> random_clientA;
            std::pair<int, int> random_clientB;
            int attempt_A = 0;
            
            const int max_attempts_A = 20;
            const int max_attempts_B = 250;

            bool no_space = true;
            do
            {
                
                random_clientA = get_random_client(s);
                int clientA_demand = s_prime[random_clientA.first].customers[random_clientA.second].demand; 
                int attempt_B = 0;
                no_space = true;

                do
                {
                    random_clientB = get_random_client(s);
                    int clientB_demand = s_prime[random_clientB.first].customers[random_clientB.second].demand; // demand klienta B

                    if ((clientA_demand > s_prime[random_clientB.first].remaining_capacity + clientB_demand) || (clientB_demand > s_prime[random_clientA.first].remaining_capacity + clientA_demand))
                    {
                        no_space = true; // nie ma miejsca na przestawienie klienta
                    }
                    else
                    {
                        no_space = false; // jest miejsce na przestawienie klienta
                    }
                    attempt_B++;
                } while ((random_clientB.first == random_clientA.first || no_space) && attempt_B < max_attempts_B);
                attempt_A++;
            } while (no_space && attempt_A < max_attempts_A);


			//jezelie nie znaleziono zamiany nie naruszajacej pojemnosci to koniec p3
            if (no_space)
            {
                std::cout << "NIE ma poprawy w P3 - brak miejsca \n";
				return s;
            }

            //KROK I - zamiana pozycji klintow A B 
            s_prime[random_clientA.first].remaining_capacity += s_prime[random_clientA.first].customers[random_clientA.second].demand;
            s_prime[random_clientB.first].remaining_capacity += s_prime[random_clientB.first].customers[random_clientB.second].demand;
            s_prime[random_clientA.first].remaining_capacity -= s_prime[random_clientB.first].customers[random_clientB.second].demand;
            s_prime[random_clientB.first].remaining_capacity -= s_prime[random_clientA.first].customers[random_clientA.second].demand;

            std::swap(s_prime[random_clientA.first].customers[random_clientA.second], s_prime[random_clientB.first].customers[random_clientB.second]);
             //kopia - moze przeniesc to kopiowanie nizej jak juz oblicze te 
            //nowe koszty ??


            double A_init_cost = g(s_prime[random_clientA.first]);
            double B_cost      = g(s_prime[random_clientB.first]);

		//dodane koszt a-init
			/*s_best[random_clientA.first].route_cost = A_init_cost;
			s_best[random_clientB.first].route_cost = B_cost;
            */

            s_best = s_prime;

            //KROK II - przestawiamy klienta A na wszystkie mozliwe miejsca w innych trasach - z wyj¹tkiem jego starej trasy i aktualnej 

            //oryginalny koszt trasy A - czyli tej ktorej bedzimy przestawiac klienta z kazdym innym
            
            //przechodzenie przez kolejne pojazdy
            double best_delta = -std::numeric_limits<double>::max();
            for (int p = 0; p < s_prime.size(); p++)
            {
                if (p == random_clientA.first || p == random_clientB.first)
                {
					continue; // nie przestawiamy w obrêbie starej i nowej trasy 
                }
               
                //oryginalny koszt trasy p
                double p_init_cost = g(s_prime[p]);
                double summary_init_cost = A_init_cost + B_cost + p_init_cost; // suma kosztow przed przestawieniem klienta A na trase p
                //przechodzenie przez wszystkich klientow w danej trasie
                for (int c = 1; c < s_prime[p].customers.size(); c++)
                {
  
                    Route A_route = s_prime[random_clientA.first]; // trasa A z klientem ktorego przestawiamy
                    Route p_route = s_prime[p];

                    int clientA_demand = A_route.customers[random_clientA.second].demand;
                    int clientp_demand = p_route.customers[c].demand;

                    if (!(clientA_demand <= p_route.remaining_capacity + clientp_demand &&
                        clientp_demand <= A_route.remaining_capacity + clientA_demand))
                    {
                        continue; // naruszenie ograniczenia pojemnosci
                    }


                    std::swap(A_route.customers[random_clientA.second], p_route.customers[c]);
                    

                    double A_new_cost = g(A_route); // nowy koszt trasy A po przestawieniu klienta na pozycje c w trasie p
                    double p_new_cost = g(p_route); // nowy koszt trasy p po przestawieniu klienta A na pozycje c w trasie p
                    double delta = summary_init_cost - (A_new_cost + B_cost + p_new_cost);

                    if (delta > best_delta) // jest poprawa
                    {
                        
                        s_best = s_prime; // przywrocenie do stanu wejsciowego 


                        best_delta = delta;
                        s_best[random_clientA.first] = A_route; // aktualizujemy trase A
                        s_best[p] = p_route; // aktualizujemy trase p
                        s_best[random_clientA.first].route_cost = A_new_cost;
                        s_best[p].route_cost = p_new_cost;

						s_best[random_clientA.first].remaining_capacity += clientA_demand;
						s_best[p].remaining_capacity -= clientA_demand;

                        s_best[random_clientA.first].remaining_capacity -= p_route.customers[c].demand;
                        s_best[p].remaining_capacity += p_route.customers[c].demand;
                    }
                }
            }
            return s_best;
        }
        default:
            break;
        }
    }

    std::vector<Route> Skewed_VNS::VND(std::vector<Route>& s_initial)
    {
        int k = 1;
        std::vector<Route> s = s_initial;
        std::vector<Route> s_prim;
        while (k <= 3)
        {
            s_prim = N(s, k);
            if (f(s) > f(s_prim))
            {
                s = s_prim;
                k = 1;
            }
            else
            {
                k = k + 1;
            }        
        }
        return s;
    }




