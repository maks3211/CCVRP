#include "skewed_vns.h"



    Skewed_VNS::Skewed_VNS(CVRPInstance instance, int num_vehicles, SkewedVNSConfig config): instance(instance), num_vehicles(num_vehicles), config(config)
{}

    //raczej nazwa run
    void Skewed_VNS::run()
    {
        std::vector<Route> routes = constructive_heurestic();
        for (int i = 0; i < routes.size(); i++)
        {
            routes[i].route_cost = g(routes[i]); // oblicz koszt trasy
        }
        result.routes = SVNS(routes);
        double result_cost = 0.0;
        for (int i = 0; i < result.routes.size(); i++)
        {
            result_cost += result.routes[i].route_cost;
        }
  
		result.total_cost = result_cost;
    }

    const Result& Skewed_VNS::get_result() const {
        return result;
    }

    std::vector<Route> Skewed_VNS::constructive_heurestic()
    {
        std::vector<std::pair<double, int>> distances_from_depot = get_all_distances(instance.depot_id - 1, instance);
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
            c.add_customer(instance.nodes[best.second], best.first, 1);
            indexes_to_remove.push_back(best.second);
        }
        //usuwanie dodanych klientow z listy wszstkich klientów
        std::sort(indexes_to_remove.begin(), indexes_to_remove.end(), std::greater<int>());
        for (int index : indexes_to_remove) {
            if (index >= 0 && index < instance.nodes.size()) {
                instance.nodes.erase(instance.nodes.begin() + index);
            }
        }
        //zwolnie pamiêci
        std::vector<int>().swap(indexes_to_remove);

        //wstawienie pozostalych klientow
        //usuniecie magazynu z listy klientów
        instance.nodes.erase(instance.nodes.begin());
        for (int i = instance.nodes.size() - 1; i >= 0; i--)
        {
            InsertionResult best_feasible_insertion;
            InsertionResult best_any_insertion;
            best_feasible_insertion.cost = std::numeric_limits<double>::infinity();
            best_any_insertion.cost = std::numeric_limits<double>::infinity();
            for (int r = 0; r < routes.size(); r++)
            {
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
            }
            InsertionResult chosen_insertion = (best_feasible_insertion.cost < std::numeric_limits<double>::infinity())
                ? best_feasible_insertion
                : best_any_insertion;

            routes[chosen_insertion.route_id].add_customer(instance.nodes[i], chosen_insertion.cost, chosen_insertion.place, false);
            instance.nodes.erase(instance.nodes.begin() + i);
        }
        return routes;
    }


    double Skewed_VNS::f(std::vector<Route>& s)
    {

        double result = 0.0;
        double capacity_penalty = 0.0;
        int m = s.size(); // number of routes/ cars
        for (int k = 0; k < m; k++)
        {
            result += g(s[k]);               //max(0, s[k].initial capacity - s[k].ile ma zaladowane = ile zostalo miejsca)
            capacity_penalty += std::max(0, s[k].remaining_capacity);
        }
        capacity_penalty *= config.f_alfa;
        result += capacity_penalty; // penelty for capacity violations
        return result;
    }

    double Skewed_VNS::delta(std::vector<Route>& s1, std::vector<Route>& s2)
    {
        int n = s1.size() - 1; //tyle sprawdzamy ile jest nastepcow 
        return 1.0 + (config.delta_alfa * (static_cast<double>(dif(s1, s2)) / n));
    }

    std::vector<Route> Skewed_VNS::SVNS(std::vector<Route>& solution, int k_max)
    {
        float beta = 1.02;
        int k = 1;
        std::vector<Route> s = solution; // operujemy na kopi
        std::vector<Route> s_best = s; // == s* w opisie algorytmu

        std::vector<Route> s_prim;
        std::vector<Route> s_bis;



        while (k <= k_max)
        {
            s_prim = s;


            for (int i = 1; i <= k; i++)
            {
                s_prim = P(s_prim, k); // wywolanie P1, P2 lub P3
            }

            s_bis = VND(s_prim);
            double f_s = f(s);
            double f_s_bis = f(s_bis);
            double delta_s_s_bis = delta(s, s_bis);
            double f_s_best = f(s_best);
            if (f_s_bis < std::min(f_s * delta_s_s_bis, beta * f_s_best))
            {
                s = s_bis;
                k = 1;
            }
            else
            {
                k = k + 1;
            }
            if (f_s_bis < f_s_best)
            {
              //  std::cout << "Jest poprawa \n";
                s_best = s_bis;
            }
            else
            {
                //std::cout << "Brak poprawy \n";
            }
        }
        return s_best;
    }

    std::vector<Route> Skewed_VNS::N(std::vector<Route>& s, int neighborhood)
    {
        std::vector<Route> s_prime = s;
        double f_s = f(s);

        switch (neighborhood) {
        case 1:
        { //iteracja przez wyszstkie pojazdy
            for (int r = 0; r < s.size(); r++)
            {
                //iteracja przez wszystkich klientow - intra route - nie wychodze poza dana trase, poza baza, ona nie moze byc przestawiona
                for (int c = 1; c < s[r].customers.size(); c++)
                {
                    for (int i = 1; i < s[r].customers.size(); i++) // miejsce za ktore wstawic klienta c, czyli np i = 2 to c bedzie na pozycji 3 
                    {
                        if (c == i) continue; // nie przestawiamy klienta na jego aktualna pozycje
                        auto client = s_prime[r].customers[c]; // klient ktorego przestawiam


                        s_prime[r].customers.erase(s_prime[r].customers.begin() + c);
                        s_prime[r].customers.insert(s_prime[r].customers.begin() + i, client); // wstawiam klienta na miejsce i

                        if (f_s > f(s_prime))
                        {
                            return s_prime; // zwracam pierwsze lepsze s_prime
                        }
                        else // nie ma poprawy to wrc do oryginalnego rozwiazania
                        {
                            s_prime[r].customers.erase(s_prime[r].customers.begin() + i);
                            s_prime[r].customers.insert(s_prime[r].customers.begin() + c, client);
                        }
                    }
                }
            }

            //inter szukanie poprawy poprzez zamiane miedzy roznymi pojazdami
            for (int r = 0; r < s.size(); r++)
            {
                for (int rr = 0; rr < s.size(); rr++)
                {
                    if (r == rr) continue;

                    //iteracja przez kolejnych klientow w trasie r
                    for (int i = 1; i < s[r].customers.size(); i++)
                    {
                        for (int ii = 1; ii < s[rr].customers.size(); ii++)
                        {
                            auto client = s_prime[r].customers[i]; // klient ktorego przestawiam
                            s_prime[r].remaining_capacity += client.demand; // przywracamy pojemnosc pojazdu, bo klient zostal usuniety
                            s_prime[r].customers.erase(s_prime[r].customers.begin() + i); // usuwam klienta ktorego przestawiam

                            s_prime[rr].customers.insert(s_prime[rr].customers.begin() + ii, client); // wstawiam klienta do innego pojazdu na miejsce ii
                            s_prime[rr].remaining_capacity -= client.demand; // przywracamy pojemnosc pojazdu, bo klient zostal usuniety
                            if (f_s > f(s_prime))
                            {
                                return s_prime; // zwracam pierwsze lepsze s_prime
                            }
                            else // nie ma poprawy to wrc do oryginalnego rozwiazania
                            {
                                s_prime[rr].remaining_capacity += client.demand; // przywracamy pojemnosc pojazdu, bo klient zostal usuniety
                                s_prime[r].remaining_capacity -= client.demand; // przywracamy pojemnosc pojazdu, bo klient zostal usuniety
                                s_prime[rr].customers.erase(s_prime[rr].customers.begin() + ii); // usuwam wstawionego klienta z innego pojazdu
                                s_prime[r].customers.insert(s_prime[r].customers.begin() + i, client); //wstawiam klienta z powrotem do oryginalnego pojazdu
                            }
                        }
                    }
                }
            }
            return s_prime;
            break;
        }
        case 2:
        {
            //iteracja przez wyszstkie pojazdy

            for (int r = 0; r < s.size(); r++)
            {
                //iteracja przez wszystkich klientow - intra route - nie wychodze poza dana trase, poza baza, ona nie moze byc przestawiona
				int block_size = config.block_size; // rozmiar bloku do przestawienia
                if (s[r].customers.size() <= config.block_size)
                {
					block_size = s[r].customers.size() > 1 ? s[r].customers.size() - 1 : 0;
                }

                for (int c = 1; c <= s[r].customers.size() - block_size; c++)
                {
                   

                    std::vector<Node> block(s_prime[r].customers.begin() + c, s_prime[r].customers.begin() + c + block_size);

                    for (int i = 1; i <= s[r].customers.size() - block_size; i++)
                    {
                        if (c == i) continue;
                        s_prime[r].customers.erase(s_prime[r].customers.begin() + c, s_prime[r].customers.begin() + c + block_size);
                        // int insertPos = (i > c) ? i - block_size : i;
                        s_prime[r].customers.insert(s_prime[r].customers.begin() + i, block.begin(), block.end());

                        if (f_s > f(s_prime))
                        {
                            return s_prime; // zwracam pierwsze lepsze s_prime
                        }
                        else // nie ma poprawy to wrc do oryginalnego rozwiazania
                        {
                            s_prime[r].customers.erase(s_prime[r].customers.begin() + i, s_prime[r].customers.begin() + i + block_size);
                            s_prime[r].customers.insert(s_prime[r].customers.begin() + c, block.begin(), block.end());
                        }

                    }
                }
            }

            //inter szukanie poprawy poprzez zamiane miedzy roznymi pojazdami
            for (int r = 0; r < s.size(); r++)
            {
                for (int rr = 0; rr < s.size(); rr++)
                {
                    if (r == rr) continue;


                    int block_size = config.block_size; // rozmiar bloku do przestawienia
                    if (s[r].customers.size() <= config.block_size)
                    {
                        block_size = s[r].customers.size() > 1 ? s[r].customers.size() - 1 : 0;
                    }
                    //iteracja przez kolejnych klientow w trasie r
                    for (int c = 1; c <= s[r].customers.size() - block_size; c++)
                    {
                        std::vector<Node> block(s_prime[r].customers.begin() + c, s_prime[r].customers.begin() + c + block_size);
                        for (int i = 1; i <= s[rr].customers.size(); i++)
                        {
                            double block_demand = 0;
                            for (const auto& client : block) {
                                block_demand += client.demand;
                            }
                            s_prime[r].remaining_capacity += block_demand;
                            s_prime[r].customers.erase(s_prime[r].customers.begin() + c, s_prime[r].customers.begin() + c + block_size);
                            s_prime[rr].customers.insert(s_prime[rr].customers.begin() + i, block.begin(), block.end());
                            s_prime[rr].remaining_capacity -= block_demand;       
                            if (f_s > f(s_prime))
                            {
                                return s_prime; // zwracam pierwsze lepsze s_prime
                            }
                            else // nie ma poprawy to wrc do oryginalnego rozwiazania
                            {
                                s_prime[rr].customers.erase(s_prime[rr].customers.begin() + i, s_prime[rr].customers.begin() + i + block_size);
                                s_prime[rr].remaining_capacity += block_demand;

                                s_prime[r].customers.insert(s_prime[r].customers.begin() + c, block.begin(), block.end());
                                s_prime[r].remaining_capacity -= block_demand;
                            }
                        }
                    }
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
                //iteracja przez wszystkich klientow - intra route - nie wychodze poza dana trase, poza baza, ona nie moze byc przestawiona
                for (int c = 1; c < s[r].customers.size() - 1; c++)
                {
                    for (int i = c + 1; i < s[r].customers.size(); i++)
                    {
                        std::swap(s_prime[r].customers[c], s_prime[r].customers[i]);
                        if (f_s > f(s_prime))
                        {
                            return s_prime; // zwracam pierwsze lepsze s_prime
                        }
                        else // nie ma poprawy to wrc do oryginalnego rozwiazania
                        {
                            std::swap(s_prime[r].customers[c], s_prime[r].customers[i]);
                        }
                    }
                }
            }
            //inter szukanie poprawy poprzez zamiane miedzy roznymi pojazdami
            for (int r = 0; r < s.size() - 1; r++)
            {
                for (int rr = r + 1; rr < s.size(); rr++)
                {
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

                            if (f_s > f(s_prime))
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
                }
            }
            return s_prime;
            break;
        }
        default:
            break;
        }
    }

    std::vector<Route> Skewed_VNS::P(std::vector<Route>& s, int procedure)
    {
        std::vector<Route> s_prime = s;
        std::vector<Route> s_best = s;
        double f_s = f(s); // koszt oryginalnego rozwiazaia



        switch (procedure)
        {

            //P1 - przestawianie w obrebie jednej trasy klienta na wszystkie mozliwe miejsca w tej trasie i wybranie najelpeszego
        case 1:
        {
            std::pair<int, int> random_client = get_random_client(s);

            auto& route = s_prime[random_client.first];
            auto client = route.customers[random_client.second];


            //zapisujemy orginalny koszt trasy ktora bedziemy modyfikowac
            double best_cost = g(route);
            int best_pos = -1;

            route.customers.erase(route.customers.begin() + random_client.second);
            for (int p = 1; p <= route.customers.size(); p++)
            {
                if (p == random_client.second)
                {
                    continue;
                }

                auto temp_customers = route.customers;
                temp_customers.insert(temp_customers.begin() + p, client);

                Route temp_route = route;
                temp_route.customers = temp_customers;
                //obliczamy nowy koszt trasy
                double current_cost = g(temp_route);
                if (current_cost < best_cost) {
                    best_cost = current_cost;
                    best_pos = p; // zapisujemy najlepsza pozycje

                }
            }
            if (best_pos != -1) {
                route.customers.insert(route.customers.begin() + best_pos, client);
                route.route_cost = best_cost; // aktualizujemy koszt trasy
            }
            else
            {
                return s;
            }
            return s_prime;
        }
        case 2:
        {
            //Szukamy najlepszej pozycja poza aktualna trasa
            std::pair<int, int> random_client = get_random_client(s);

            auto client = s_prime[random_client.first].customers[random_client.second]; // losowy klient ktorego przestawiam


            auto& first_route = s_prime[random_client.first];
            first_route.remaining_capacity += client.demand;
            first_route.customers.erase(first_route.customers.begin() + random_client.second);


            double first_route_init_cost = s[random_client.first].route_cost;
            double first_route_removed_cost = g(first_route); //nowy koszt trasy z ktorej usuwamy klienta

            double best_delta = 0.0;


            for (int r = 0; r < s.size(); r++)//przechodzimy przez kolejne trasy
            {

                if (r == random_client.first)
                {
                    continue; // nie przestawiamy klienta w tej samej trasie
                }
                auto& second_route = s_prime[r];
                // Sprawdzenie pojemnoœci raz dla ca³ej trasy
                if (client.demand > second_route.remaining_capacity)
                    continue;

                double second_route_init_cost = s[r].route_cost;

                for (int c = 1; c <= s[r].customers.size(); c++)
                {
                    auto temp_customers = second_route.customers;
                    temp_customers.insert(temp_customers.begin() + c, client);

                    // trasa z nowym uk³adem klientów i zaktualizowan¹ pojemnoœci¹
                    Route temp_route = second_route;
                    temp_route.customers = temp_customers;
                    temp_route.remaining_capacity -= client.demand;

                    double second_route_removed_cost = g(temp_route);

                    double cost_delta = (first_route_init_cost + second_route_init_cost) - (first_route_removed_cost + second_route_removed_cost);
                    if (cost_delta > best_delta) // oznacza ze poprawa
                    {
                        //  std::cout << "P2 POPRAWA \n";
                        best_delta = cost_delta;
                        s_best = s_prime;
                        s_best[r] = temp_route;
                        s_best[random_client.first] = first_route;

                        s_best[random_client.first].route_cost = first_route_removed_cost;
                        s_best[r].route_cost = second_route_removed_cost;
                    }
                }
            }

            if (best_delta > 0.0)
            {
                return s_best;
            }
            return s;

        }
        case 3:
        { // P3 wymianiamy dwoch losowych klientow z losowych tras nastepnie przestawiamy klienta z pierwszej trasy na kazda inna pozycje w kazdej innej trasie poza trasa 
            std::pair<int, int> random_clientA = get_random_client(s);
            std::pair<int, int> random_clientB;
            int attempt = 0;
            const int max_attempts = 500; //!!!!!!!!Moze wyznaczaæ to jakoœ m¹drzej np. na podstawie ilosci klientow,  maksymalna liczba prób, aby unikn¹æ nieskoñczonej pêtli
            bool no_space = true;
            do
            {
                random_clientB = get_random_client(s);
                int clientA_demand = s_prime[random_clientA.first].customers[random_clientA.second].demand; // demand klienta A
                int clientB_demand = s_prime[random_clientB.first].customers[random_clientB.second].demand; // demand klienta B

                if ((clientA_demand > s_prime[random_clientB.first].remaining_capacity + clientB_demand) || (clientB_demand > s_prime[random_clientA.first].remaining_capacity + clientA_demand))
                {
                    no_space = true; // nie ma miejsca na przestawienie klienta
                }
                else {
                    no_space = false; // jest miejsce na przestawienie klienta
                }
                attempt++;
            } while ((random_clientB.first == random_clientA.first || no_space) && attempt < max_attempts);

            //KROK I - zamiana pozycji klintow A B 
            s_prime[random_clientA.first].remaining_capacity += s_prime[random_clientA.first].customers[random_clientA.second].demand;
            s_prime[random_clientB.first].remaining_capacity += s_prime[random_clientB.first].customers[random_clientB.second].demand;
            s_prime[random_clientA.first].remaining_capacity -= s_prime[random_clientB.first].customers[random_clientB.second].demand;
            s_prime[random_clientB.first].remaining_capacity -= s_prime[random_clientA.first].customers[random_clientA.second].demand;

            std::swap(s_prime[random_clientA.first].customers[random_clientA.second], s_prime[random_clientB.first].customers[random_clientB.second]);
            s_best = s_prime; //kopia 

            //KROK II - przestawiamy klienta A na wszystkie mozliwe miejsca w innych trasach - chyba tez mozna przestawiac w obrebie tej samej trasy 

            //oryginalny koszt trasy A - czyli tej ktorej bedzimy przestawiac klienta z kazdym innym
            double A_init_cost = g(s_prime[random_clientA.first]);
            //przechodzenie przez kolejne pojazdy
            for (int p = 0; p < s_prime.size(); p++)
            {
                double best_delta = 0.0;
                //oryginalny koszt trasy p
                double p_init_cost = g(s_prime[p]);
                double summary_init_cost = A_init_cost + p_init_cost; // suma kosztow przed przestawieniem klienta A na trase p
                //przechodzenie przez wszystkich klientow w danej trasie
                for (int c = 1; c < s_prime[p].customers.size(); c++)
                {

                    Route A_route = s_prime[random_clientA.first]; // trasa A z klientem ktorego przestawiamy
                    Route p_route = s_prime[p];

                    if (p == random_clientA.first) // jezli przestawamy w obrebie tej samej trasy 
                    {
                        std::swap(A_route.customers[random_clientA.second], A_route.customers[c]);
                        p_route = A_route;
                    }
                    else
                    {
                        std::swap(A_route.customers[random_clientA.second], p_route.customers[c]);
                    }

                    double A_new_cost = g(A_route); // nowy koszt trasy A po przestawieniu klienta na pozycje c w trasie p
                    double p_new_cost = g(p_route); // nowy koszt trasy p po przestawieniu klienta A na pozycje c w trasie p
                    double delta = summary_init_cost - (A_new_cost + p_new_cost);

                    if (delta > best_delta) // jest poprawa
                    {
                        s_best = s_prime; // przywrocenie do stanu wejsciowego 

                        best_delta = delta;

                        // std::cout << "P3 POPRAWA \n";
                        s_best[random_clientA.first] = A_route; // aktualizujemy trase A
                        s_best[p] = p_route; // aktualizujemy trase p
                        s_best[random_clientA.first].route_cost = A_new_cost;
                        s_best[p].route_cost = p_new_cost;
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




