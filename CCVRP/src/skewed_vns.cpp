#include "skewed_vns.h"



    Skewed_VNS::Skewed_VNS(CVRPInstance instance, int num_vehicles, IO_handlerV2::IO_handler io_handlers_v2, SkewedVNSConfig config): instance(instance),io_handlers_v2(io_handlers_v2), num_vehicles(num_vehicles), config(config)
    {
      
    }

    void Skewed_VNS::run()
    {

        std::vector<Route> routes = constructive_heurestic();
        
        //for (int i = 0; i < routes.size(); i++)
       // {
           f_new(routes); // oblicz koszt trasy
        //}
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



    double Skewed_VNS::f_new(std::vector<Route>& s, const std::vector<int>& changed_routes)
    {

        double total_result = 0.0;
        double total_capacity_penalty = 0.0;
        double capacity_penalty = 0.0;
        int m = s.size(); // number of routes/ cars

        //przeliczanie wszystkich tras
        if (changed_routes.empty())
        {
            for (int k = 0; k < m; k++)
            {
                //double cost = g(s[k]);
                //s[k].route_cost = cost;
				g(s[k]);  // przeliczenie kosztu trasy, g() od razu wpisuje ten nowy koszt do pola route_cost danej trasy
                capacity_penalty = std::abs(std::min(0, s[k].remaining_capacity)); 
                if (capacity_penalty > 0)
                {
					s[k].route_cost += config.f_alfa * capacity_penalty; // dodanie kary 
                }


            }
        }

        //przeliczanie tylko zmienionych tras - reszta korzysta z pola route_cost
        else
        {
            for (int k : changed_routes)
            {
                g(s[k]);
                capacity_penalty = std::abs(std::min(0, s[k].remaining_capacity));
                if (capacity_penalty > 0)
                {
                    s[k].route_cost += config.f_alfa * capacity_penalty; // dodanie kary 
                }
            }

        }

        for (const auto& route : s)
        {
            total_result += route.route_cost;

        }
        return total_result;
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
            while (k <= k_max) //linia 5        Algorithm 3
            {
                s_prim = s;
 
                //?????
                calculate_remaining_capacity(s);
				//TAK BYLO NA POCZATKU
                for (int i = 1; i <= k; i++)
                {
                    int random_perturbation_proc = random_int_from_to(1, 3);
                   // s_prim = P(s_prim, k); //perturbation
                    s_prim = P(s_prim, random_perturbation_proc); //perturbation    //linia 6            // stare rozwiazanie ->  s_test = P(s_prim, k);
                }
               
           
              
                //double przed_P = calculate_cost(s_prim); //to bylo

             
				
				
              
				//s_prim = s_test;
				double po_P = calculate_cost(s_prim);

                calculate_remaining_capacity(s_prim);
                s_bis = VND(s_prim);        //local search      //linia 7
				double po_VND = calculate_cost(s_bis);

                double f_s = f_new(s);
                double f_s_bis = f_new(s_bis);
                double delta_s_s_bis = delta(s, s_bis);
            
                double f_s_best = f_new(s_best);
                if (f_s_bis < std::min(f_s * delta_s_s_bis, beta * f_s_best))  //skewed move        //linia 8
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
        double f_s = f_new(s);
        std::vector<Route> s_prime = s;
   
		


        switch (neighborhood) {
        case 1:
        { //iteracja przez wyszstkie pojazdy
            for (int r = 0; r < s.size(); r++)
            {
                //iteracja przez wszystkich klientow - intra route - nie wychodze poza dana trase, poza baza, ona nie moze byc przestawiona
                //Zminaa c z 1 na 0

				std::vector<int> changed_routes = {r};

                int route_size = s_prime[r].customers.size();
                for (int c = 1; c < route_size; c++)
                {
                    auto client = s_prime[r].customers[c]; // klient ktorego przestawiam
                    s_prime[r].customers.erase(s_prime[r].customers.begin() + c);

                    for (int i = 1; i < route_size; i++) // miejsce za ktore wstawic klienta c, czyli np i = 2 to c bedzie na pozycji 3 
                    {
                        if (c == i) continue; // nie przestawiamy klienta na jego aktualna pozycje
                        s_prime[r].customers.insert(s_prime[r].customers.begin() + i, client); // wstawiam klienta na miejsce i
                        if (f_s > f_new(s_prime, changed_routes))
                        {          
                            return s_prime; // zwracam pierwsze lepsze s_prime
                        }
						// brak poprawy - cofniece proby
                        s_prime[r].customers.erase(s_prime[r].customers.begin() + i);
                        
                    }
					//powrot do oryginalnej pozycji klienta
                    s_prime[r].customers.insert(s_prime[r].customers.begin() + c, client);
					//ponowne przeliczenie kosztu trasy po przywrocenie jej stanu
                    f_new(s_prime, changed_routes);
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
                            s_prime[rr].remaining_capacity -= client.demand; // przywracamy pojemnosc pojazdu, bo klient zostal wstawiony
                            if (f_s > f_new(s_prime, changed_routes))
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
                        f_new(s_prime, changed_routes);
                    }
                    f_new(s_prime, changed_routes);
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

                        if (f_s > f_new(s_prime, changed_routes))
                        {                  
                            return s_prime; // zwracam pierwsze lepsze s_prime
                        }
                            // nie ma poprawy to wrc do oryginalnego rozwiazania
                            s_prime[r].customers.erase(s_prime[r].customers.begin() + i, s_prime[r].customers.begin() + i + block_size);
                           // s_prime[r].customers.insert(s_prime[r].customers.begin() + c, block.begin(), block.end());
                           // s_prime = s_tmp;
                    }
                    s_prime[r].customers.insert(s_prime[r].customers.begin() + c, block.begin(), block.end());
                    f_new(s_prime, changed_routes);
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
                            if (f_s > f_new(s_prime, changed_routes))
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
                    f_new(s_prime, changed_routes);
                }
                
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
                        if (f_s > f_new(s_prime, changed_routes))
                        {  
                            return s_prime; // zwracam pierwsze lepsze s_prime
                        }
                        else // nie ma poprawy to wrc do oryginalnego rozwiazania
                        {
                            std::swap(s_prime[r].customers[c], s_prime[r].customers[i]);
                        }
                    }
                }
                f_new(s_prime, changed_routes);
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

                            if (f_s > f_new(s_prime, changed_routes))
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
                    f_new(s_prime, changed_routes);
                }
               
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
     
                std::pair<int, int> random_client = get_random_client(s);
                auto& route = s_prime[random_client.first];

                if (route.customers.size() <= 2) continue;
                int k = random_client.second;
                Node client = route.customers[k]; // wylosowany klient dla ktorego sprawdzam wszystkie pozycje w jego trasie
                double best_f = std::numeric_limits<double>::max();
                int best_pos = -1;

                route.customers.erase(route.customers.begin() + k);

                std::vector<int> changed_routes = { random_client.first };
                for (int pos = 1; pos <= route.customers.size(); ++pos)
                {
                    if (pos == k) continue; // nie przestawiam na to samo miejsce 
					route.customers.insert(route.customers.begin() + pos, client);
                    double current_f = f_new(s_prime, changed_routes);
                    if (current_f < best_f)
                    {
                        best_f = current_f;
                        best_pos = pos;
                    }
					route.customers.erase(route.customers.begin() + pos);
                }

                //zapisanie wyniku
                if (best_pos != -1 ) {

                    route.customers.insert(route.customers.begin() + best_pos, client);
                    f_new(s_prime, changed_routes);
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
            double sol_cost = 0.0;
			double best_f = std::numeric_limits<double>::max(); //suma kosztow obu tras po przestawieniu klienta
            for (int max_attempts = 0; max_attempts < 20; max_attempts++)
            {
               
                //Szukamy najlepszej pozycja poza aktualna trasa
                std::pair<int, int> random_client = get_random_client(s);
                Route& r1 = s_prime[random_client.first];
              
                Node node_to_move = r1.customers[random_client.second]; // losowy klient ktorego przestawiam
                if (r1.customers.size() < 2) continue;
   
                int k = random_client.second;
                r1.customers.erase(r1.customers.begin() + k); //usun klienta z trasy
				r1.remaining_capacity += node_to_move.demand; // zwolnij pojemnosc

				f_new(s_prime, { random_client.first }); 
			
                for (int r = 0; r < s.size(); r++)//przechodzimy przez kolejne trasy
                {

                    if (r == random_client.first)
                    {
                        continue; // nie przestawiamy klienta w tej samej trasie
                    }

                    Route& r2 = s_prime[r];
                    r2.remaining_capacity -= node_to_move.demand; // zajmij pojemnosc w nowej trasie
                    for (int c = 1; c <= r2.customers.size(); c++) // przejscie przez wszyskie pozycje w innej trasie
                    {
                        r2.customers.insert(r2.customers.begin() + c, node_to_move);
						

                        sol_cost =  f_new(s_prime, { r });

             
                        if (best_f > sol_cost) // poprawa
                        {
                            //  std::cout << "P2 POPRAWA \n";
                            best_f = sol_cost;
                            best_pos = c; //pozycja  c w trasie r na ktora wstawic klienta
                            best_route = r;
                            
                        }
                        r2.customers.erase(r2.customers.begin() + c);
                       
                    }
                    r2.remaining_capacity += node_to_move.demand;
                }
                if (best_pos == 0)
                {
                    continue;
                }
                //ZAPISANIE WYNIKU

                //std::cout << "\n pozzucja:" << best_pos << " rozmiar trasy: ";
                //std::cout << s_prime[best_route].customers.size() << "\n";
                s_prime[best_route].customers.insert(s_prime[best_route].customers.begin() + best_pos, node_to_move);
                s_prime[best_route].remaining_capacity -= node_to_move.demand;
				f_new(s_prime, { best_route });
                
                return s_prime; // zwraca najlepsze rozwiazanie - usuwa losowego klienta i wstawia go na najlepsza pozycje
            }
            return s; // w ogle nie udalo sie znalezc poprawy w max_attempts - zwroc orginal

        }
        case 3:
		{ // P3 wymiana losowego klienta z innym klientem z innej trasy - szukanie poprawy w tej wymianie - czyli zwykly swap 
            std::pair<int, int> random_clientA;
            int best_route = 0, best_pos = 0;
            double sol_cost = 0.0;
            int attempt_A = 0;
            
            const int max_attempts_A = 20;
            const int max_attempts_B = 250;

            bool no_space = true;
            random_clientA = get_random_client(s);
			Route &random_routeA = s[random_clientA.first];
			Node &clientA = random_routeA.customers[random_clientA.second];

            double best_f = std::numeric_limits<double>::max();
            double current_f = 0.0;
           
            for(int r = 0; r < s_prime.size(); ++r)
            {
                if (r == random_clientA.first)
                {
					continue; // nie przestawiamy klienta w tej samej trasie
                }

				for (int c = 1; c < s_prime[r].customers.size(); ++c) // przejscie przez wszyskie pozycje w innej trasie
                {                  
                    s_prime[r].remaining_capacity += s_prime[r].customers[c].demand;
                    s_prime[r].remaining_capacity -= clientA.demand;
                    random_routeA.remaining_capacity += clientA.demand;
                    random_routeA.remaining_capacity -= s_prime[r].customers[c].demand;

                    std::swap(s_prime[random_clientA.first].customers[random_clientA.second], s_prime[r].customers[c]);
                    current_f = f_new(s_prime, { random_clientA.first, r });
                    if (current_f < best_f)
                    {
                        best_f = current_f;
                        best_route = r;
						best_pos = c;
                    }

                    std::swap(s_prime[random_clientA.first].customers[random_clientA.second], s_prime[r].customers[c]);
                    s_prime[r].remaining_capacity -= s_prime[r].customers[c].demand;
                    s_prime[r].remaining_capacity += clientA.demand;
                    random_routeA.remaining_capacity -= clientA.demand;
                    random_routeA.remaining_capacity += s_prime[r].customers[c].demand;

				}
            }


            s_prime[best_route].remaining_capacity += s_prime[best_route].customers[best_pos].demand;
            s_prime[best_route].remaining_capacity -= clientA.demand;
            random_routeA.remaining_capacity += clientA.demand;
            random_routeA.remaining_capacity -= s_prime[best_route].customers[best_pos].demand;

            std::swap(s_prime[random_clientA.first].customers[random_clientA.second], s_prime[best_route].customers[best_pos]);
            f_new(s_prime, { random_clientA.first, best_route });

            return s_prime;
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
		double s_prime_f = 0.0; 
        while (k <= 3)
        {
            s_prime_f = 0.0;
            s_prim = N(s, k);
            for (auto& a : s_prim)
            {
				s_prime_f += a.route_cost;
            }
            if (f_new(s) > s_prime_f)
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




