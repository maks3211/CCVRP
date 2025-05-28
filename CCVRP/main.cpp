

#include <iostream>
#include <algorithm>
#include <random>
#include "CvrpParser.h"
#include "Route.h"
double euclidean_distance(double x1, double y1, double x2, double y2)
{
	return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}
double euclidean_distance(Node n1, Node n2)
{
    
    double res = sqrt((n1.x - n2.x) * (n1.x - n2.x) + (n1.y - n2.y) * (n1.y - n2.y));
    return res;
}

/// <summary>
/// 
/// </summary>
/// <param name="from_node"></param>
/// <param name="instance"></param>
/// <returns>Vector of pairs- distance and second node</returns>
std::vector<std::pair<double, int>> get_all_distances(const int from_node, const CVRPInstance& instance) // zwraca wektor odległości węzła from_node do pozstałych punktów
{
    if (from_node >= instance.dimension)
    {
        return{};
    }
    std::vector<std::pair<double, int>> distances;
    for (int i = 0; i < instance.dimension; i++)
    {
        if (i == from_node)
        {
            continue;
        }
        else
        {
            distances.push_back({ euclidean_distance(instance.nodes[from_node], instance.nodes[i]), i });
        }
    }
    return distances;
}


struct InsertionResult
{
    int place;

	int route_id;
	double cost;
    InsertionResult() = default;
    InsertionResult(int p,int id, double c) : place(p), route_id(id), cost(c) {}
};


InsertionResult find_best_insertion_improved(Route& route, Node& i)
{
    int place = 1;
	double cost = std::numeric_limits<double>::infinity();
    int p = route.customers.size() - 1;
    for (int j = 0; j < route.customers.size(); j++)
    {
        double currentCost = 0.0;
        double a = 0.0;
        double b = 0.0;
        double c = 0.0;
        for (int h = 0; h < j; h++)
        {
			a += euclidean_distance(route.customers[h], route.customers[h + 1]);
        }

        b = euclidean_distance(route.customers[j],i);
        int tmp = p - j;
        if (tmp <= 0)
        {
            c = 0;
        }
        else if (tmp > 0)
        {    
            c = tmp * (euclidean_distance(route.customers[j],i) + euclidean_distance(route.customers[j+1], i));
        }

		currentCost = a + b + c;    
        if (currentCost < cost)
        {
            cost = currentCost;
            place = j;
        }
    }
	return InsertionResult(place, route.vehicle_id, cost);  // place - po ktorym wstawic nowego klienta if place == 0 to nowy bedzie na idx 1 
}

//funkcja zwraca koszt calej trasy pi
double g(Route &pi)
{
	int p = pi.customers.size() - 1; // liczba klientów w trasie (minus baza)
    double result = 0.0;
    for (int i = 1; i <= p; i++)
    {
		result += (p - i + 1) * euclidean_distance(pi.customers[i-1], pi.customers[i]);
    }
    return result;
}

//Funkcja oceny rozwiazania s 
double f(std::vector<Route> &s)
{
    double alfa = 10.1;
    double result = 0.0;
    double capacity_penalty = 0.0;
    int m = s.size(); // ile pojazdow
	//suma wszystkich kosztow
    for (int k = 0; k < m; k++)
    {
		result += g(s[k]);               //max(0, s[k].initial capacity - s[k].ile ma zaladowane = ile zostalo miejsca)
        capacity_penalty += std::max(0, s[k].remaining_capacity);
    }
    capacity_penalty *= alfa;
	result += capacity_penalty; // dodajemy karę za przekroczenie pojemności
    return result;
}
/// <summary>
/// Aktualizuje ladownosc pojazdow, lecz ignoruje przekroczenie pojemnosci 
/// </summary>
/// <param name="s">Solution to permute</param>
/// <param name="neighborhood">neighborhood type allowed vales 1,2 or 3</param>
/// <returns></returns>
std::vector<Route> N(std::vector<Route>& s, int neighborhood)
{
    int block_size = 3; // rozmiar bloku w przypadku N2
    //zakladamy ze sprawdzanie rozpoczynam od pierwszego klienta z pierwszej trasy i sprawdzam po kolei
	std::vector<Route> s_prime = s; // kopia s
	double f_s = f(s); 
    switch (neighborhood) {
    case 1:
        //iteracja przez wyszstkie pojazdy
        for (int r = 0; r < s.size(); r++)
        {
            //iteracja przez wszystkich klientow - intra route - nie wychodze poza dana trase, poza baza, ona nie moze byc przestawiona
            for (int c = 1; c < s[r].customers.size() - 1; c++)
            {
                for (int i = c + 1; i < s[r].customers.size(); i++) // miejsce za ktore wstawic klienta c, czyli np i = 2 to c bedzie na pozycji 3 
                {
					auto client = s_prime[r].customers[c]; // klient ktorego przestawiam

			
                    s_prime[r].customers.erase(s_prime[r].customers.begin() + c);

					s_prime[r].customers.insert(s_prime[r].customers.begin() + i, client); // wstawiam klienta na miejsce i
                
                    //CZY JEST SENS PRZELICZAC JAKOSC CALEGO ROZIWAZANIA, MOZE LEPIEJ TYLKO TYCH TRAS KTORE SIE ZMIENILY?  W INNYCH MIEJSCACH PODOBNIE 
                    //sprawdzic jak to jest w dokumentacji, moze jednak tak trzeba??
					//no chya jednak jest  ok bo w algorithm 2 sa porownywane f i f_s_prime
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
        for (int r = 0; r < s.size() - 1; r++)
        {
            for (int rr = r + 1; rr < s.size(); rr++)
            {
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
    case 2:       
        //iteracja przez wyszstkie pojazdy
        for (int r = 0; r < s.size(); r++)
        {
            //iteracja przez wszystkich klientow - intra route - nie wychodze poza dana trase, poza baza, ona nie moze byc przestawiona
            for (int c = 1; c < s[r].customers.size() - block_size; c++)
            {
                for (int i = c + block_size; i <= s[r].customers.size(); i++) 
                {
       
                    std::vector<Node> block(s_prime[r].customers.begin() + c, s_prime[r].customers.begin() + c + block_size);
                    s_prime[r].customers.erase(s_prime[r].customers.begin() + c, s_prime[r].customers.begin() + c + block_size);
                    int insertPos = (i > c) ? i - block_size : i;
                    s_prime[r].customers.insert(s_prime[r].customers.begin() + insertPos, block.begin(), block.end());

                    if (f_s > f(s_prime))
                    {
                      return s_prime; // zwracam pierwsze lepsze s_prime
                    }
                    else // nie ma poprawy to wrc do oryginalnego rozwiazania
                    {
                        s_prime[r].customers.erase(s_prime[r].customers.begin() + insertPos, s_prime[r].customers.begin() + insertPos + block_size);
                        s_prime[r].customers.insert(s_prime[r].customers.begin() + c, block.begin(), block.end());
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
                for (int i = 1; i < s[r].customers.size() - block_size; i++)
                {
                    for (int ii = 1; ii < s[rr].customers.size(); ii++)
                    {

                        std::vector<Node> block(s_prime[r].customers.begin() + i, s_prime[r].customers.begin() + i + block_size);
                        double block_demand = 0;
                        for (const auto& client : block) {
                            block_demand += client.demand;
                        }
                        s_prime[r].remaining_capacity += block_demand;
                        s_prime[r].customers.erase(s_prime[r].customers.begin() + i, s_prime[r].customers.begin() + i + block_size);

                        s_prime[rr].customers.insert(s_prime[rr].customers.begin() + ii, block.begin(), block.end());
                        s_prime[rr].remaining_capacity -= block_demand;

                        if (f_s > f(s_prime))
                        {
                            return s_prime; // zwracam pierwsze lepsze s_prime
                        }
                        else // nie ma poprawy to wrc do oryginalnego rozwiazania
                        {    
                            s_prime[rr].customers.erase(s_prime[rr].customers.begin() + ii, s_prime[rr].customers.begin() + ii + block_size);
                            s_prime[rr].remaining_capacity += block_demand;

                            s_prime[r].customers.insert(s_prime[r].customers.begin() + i, block.begin(), block.end());
                            s_prime[r].remaining_capacity -= block_demand;
                        }
                    }
                }
            }
        }
        return s_prime;
        break;
	case 3:
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
    default:
		break;
    }

}

std::vector<Route> VND(std::vector<Route>& s_initial)
{
    int k = 1;
    std::vector<Route> s = s_initial;
    std::vector<Route> s_prim;
    while (k <= 3)
    {
        std::cout << k << "\n";
        s_prim = N(s,k);
        if ( f(s) > f(s_prim) )
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

/// <summary>
/// 
/// </summary>
/// <param name="s"></param>
/// <returns>Pair .first random route index .second random client index in .first route </returns>
std::pair<int,int> get_random_client(std::vector<Route>& s)
{
    std::random_device rd;
    std::mt19937 gen(rd());

    int random_route;
    do {
        std::uniform_int_distribution<> route_dist(0, s.size() - 1);
        random_route = route_dist(gen);
    } while (s[random_route].customers.size() <= 1);

    // Skoro trasa ma klientów, losujemy klienta (pomiń depozyt na pozycji 0)
    std::uniform_int_distribution<> client_dist(1, s[random_route].customers.size() - 1);
    int random_client = client_dist(gen);

    std::cout << "Wylosowano klienta " << random_client << " z trasy " << random_route << "\n";
    return {random_route, random_client};
}



//!!!!!PRZED ZASTOSOWANIEM TRZEBA OBLICZYC KOSZTY IMPROVED COST DLA KAZDEJ Z TRASY ZA POMOCA FUNKCJI G
std::vector<Route> P(std::vector<Route>& s, int procedure)
{

    std::vector<Route> s_prime = s;
    std::vector<Route> s_best = s;
    double f_s = f(s); // koszt orginalnego rozwiazaia
    
    switch(procedure)
    {
    case 1:
    {
        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //ZASTOSOWAC PODOBNA LOGIKE DO N1 - czyli najpierw usunac tego przestawianego i potem go przekladac zamiast za kazdym razem go usuwac z nowej pozycji i w stawiac na wejsciowej
        //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        std::pair<int, int> random_client = get_random_client(s);

        double best_cost = std::numeric_limits<double>::infinity();
        auto client = s_prime[random_client.first].customers[random_client.second];

        s_prime[random_client.first].remaining_capacity += client.demand; // przywracamy pojemnosc pojazdu, bo klient zostal usuniety
        s_prime[random_client.first].customers.erase(s_prime[random_client.first].customers.begin() + random_client.second);
        for (int p = 1; p <= s[random_client.first].customers.size(); p++)
        {
            if (p == random_client.second)
            {
                continue;
            }
            if (client.demand > s_prime[random_client.first].remaining_capacity)
            {
                continue; // nie ma miejsca w trasie, wiec nie przestawiamy klienta
            }

            auto temp_route = s_prime;
            temp_route[random_client.first].remaining_capacity -= client.demand; // zmniejszamy pojemnosc pojazdu, bo klient zostal przestawiony
            temp_route[random_client.first].customers.insert(temp_route[random_client.first].customers.begin() + p, client);
            // losowy klient ktorego przestawiam
            // 
            // !!!!!!!!!!!!!! zamienić f() na g()
            //CZY TRZEBA POROWNYWAC CALE ROZWIAZANIE, A NIE TYLKO CZY TA TRASA SIE POPRAWILA SKORO P1 DZIALA WEWNATRZ TRASY
            double current_cost = f(temp_route);


            if (current_cost < best_cost) {
                best_cost = current_cost;
                s_best = temp_route;
            }
        }
        return s_best;
        break;
    }
    case 2:
    {
		std::cout << "\nP2\n";
        std::pair<int, int> random_client = get_random_client(s);

        auto client = s_prime[random_client.first].customers[random_client.second]; // losowy klient ktorego przestawiam

        //kopia orginalu z usunietym klientem którego przestawiamy
		s_prime[random_client.first].remaining_capacity += client.demand; // przywracamy pojemnosc pojazdu, bo klient zostal usuniety
        s_prime[random_client.first].customers.erase(s_prime[random_client.first].customers.begin() + random_client.second);
        


        double first_route_init_cost = s[random_client.first].route_cost_improved;
        double first_route_removed_cost = g(s_prime[random_client.first]); //nowy koszt trasy z ktorej usuwamy klienta

        double best_delta = 0.0;
        for (int r = 0; r < s.size(); r++)//przechodzimy przez kolejne trasy
        {
            
            if (r == random_client.first)
            {
                continue; // nie przestawiamy klienta w tej samej trasie
            }
            double second_route_init_cost = s[r].route_cost_improved;

            for (int c = 1; c <= s[r].customers.size(); c++)
            {
                if (client.demand > s_prime[r].remaining_capacity)
                {
					continue; // nie ma miejsca w trasie, wiec nie przestawiamy klienta
                }


                auto temp_route = s_prime;
				temp_route[r].remaining_capacity -= client.demand; // zmniejszamy pojemnosc pojazdu, bo klient zostal przestawiony
                temp_route[r].customers.insert(temp_route[r].customers.begin() + c, client);
                



                double second_route_removed_cost = g(temp_route[r]);
                double cost_delta = (first_route_init_cost + second_route_init_cost) - (first_route_removed_cost + second_route_removed_cost);
                if (cost_delta > best_delta) // oznacza ze poprawa
                {
                    std::cout << "POPRAWA \n";
                    best_delta = cost_delta;
                    s_best = temp_route; // zapisujemy najlepsze rozwiazanie
                }
            }
        }
        //jezeli nie ma poprawy to zwracamy orginalne rozwiazanie
        if (best_delta > 0.0)
            return s_best;
        else
            return s;
        break;
    }
    case 3:
    { // P3 wymianiamy dwoch losowych klientow z losowych tras nastepnie przestawiamy klienta z pierwszej trasy na kazda inna pozycje w kazdej innej trasie poza trasa 
        std::pair<int, int> random_clientA = get_random_client(s);
        std::pair<int, int> random_clientB;
        int attempt = 0;
		const int max_attempts = 500; //!!!!!!!!Moze wyznaczać to jakoś mądrzej np. na podstawie ilosci klientow,  maksymalna liczba prób, aby uniknąć nieskończonej pętli
		bool no_space = true;
        do
        {
            random_clientB = get_random_client(s);
			int clientA_demand = s_prime[random_clientA.first].customers[random_clientA.second].demand; // demand klienta A
			int clientB_demand = s_prime[random_clientB.first].customers[random_clientB.second].demand; // demand klienta B

            if ( (clientA_demand > s_prime[random_clientB.first].remaining_capacity + clientB_demand) || (clientB_demand > s_prime[random_clientA.first].remaining_capacity + clientA_demand))
            {
				no_space = true; // nie ma miejsca na przestawienie klienta
            }
            else {
				no_space = false; // jest miejsce na przestawienie klienta
            }
        attempt++;
        }while ( (random_clientB.first == random_clientA.first || no_space) && attempt < max_attempts);

        

        //KROK I - zamiana pozycji klintow A B 
		s_prime[random_clientA.first].remaining_capacity += s_prime[random_clientA.first].customers[random_clientA.second].demand; 
		s_prime[random_clientB.first].remaining_capacity += s_prime[random_clientB.first].customers[random_clientB.second].demand; 
		s_prime[random_clientA.first].remaining_capacity -= s_prime[random_clientB.first].customers[random_clientB.second].demand; 
		s_prime[random_clientB.first].remaining_capacity -= s_prime[random_clientA.first].customers[random_clientA.second].demand; 
        std::swap(s_prime[random_clientA.first].customers[random_clientA.second], s_prime[random_clientB.first].customers[random_clientB.second]);
      
        
		//KROK II - przestawiamy klienta A na wszystkie mozliwe miejsca w innych trasach








        break;
    }
    default:
        break;
    }
}

void SVNS()
{
	//PERTURABATION PHASE P(N) TRZEBA ZROBIC
	// VND PHASE  - DONE
	//f(s) - DONE 
	//delta (s,s`) - TRZEBA ZROBIC
}



/// <summary>
/// W main wywoluje  funkcje dla wszsytkich route (jezeli bedzie dosc miejsca), i tam zapisuje min 
/// Tutaj tylko szukam najlepszego miejsca dla konkretnej trasy i konkretnego klienta
/// </summary>
/// <param name="route">Dana trasa do sprawdzenia - k/param>
/// <param name="instance">wstawiany klient - i</param>
/// <returns>Koszt wstawienia oraz miejsce wstawienia - indeks na ktory wstawic</returns>
InsertionResult find_best_insertion(Route& route, Node &i ) 
{
    int place = -1;
    double cost = std::numeric_limits<double>::infinity();
	int p = route.customers.size() - 1; // liczba klientów w trasie (minus baza)
    if (route.customers.empty())
        return InsertionResult(1, route.vehicle_id, 7);
      //  return InsertionResult(-1, route.vehicle_id, std::numeric_limits<double>::infinity());


	//iteracja przez wszystkie mozliwe miejsca wstawienia klienta
    for (int j = 1; j < route.customers.size()-1; j++)
    {
		double currentCost = 0.0;
        double second;
        double third;
        //sigma
        double sigmaCost = 0.0;
        for(int h = 0; h<= j; h++) //h <=j
        {
         //   std::cout << " jeden  " << h << "  ";
            sigmaCost += euclidean_distance(route.customers[h], route.customers[h+1]);
          
        }
      //  std::cout << " dwa  " << j;
        second = euclidean_distance(route.customers[j], i);
        
        //(p-j+1)  jest p-1 bo w tym jest juz baza ktora nie jest klientem chyba 
      //  std::cout << " trzy  ";
        third = (euclidean_distance(route.customers[j], i));
       
        if (j + 1 < route.customers.size())
        {
        //    std::cout << " cztery  " << std::endl;
            third += euclidean_distance(i, route.customers[j + 1]);
            
        }
        //TUUUUUUUUUUUUUUUUU
        if ((p - j + 1) != 0)
        {
        //    third *= (p - j + 1);
        }
		currentCost = sigmaCost + second + third;
        if (currentCost < cost)
        {
			cost = currentCost;
			place = j; // miejsce wstawienia klienta
        }
    }
    return InsertionResult(place,route.vehicle_id,cost);
}




// !!!!!!!!!!!!!NIGDZIE PRZY PRZESTAWIANIU KLIENTOW NIE AKTUALIZUJE POJEMNOSCI POJAZDOW !!!!!!!!!!!!!
int main()
{

    //instance.nodes 

    std::cout << "Hello World!\n";
    const int num_vehicles = 9;

    CVRPInstance instance = parseCVRPFile("InputData/Golden_1.vrp");
    //CVRPInstance instance = parseCVRPFile("InputData/bronzen.vrp");
    std::cout << "Nazwa instancji: " << instance.name << "\n";

    std::cout << "Liczba klientow: " << instance.dimension << "\n"; // w tym zawiera się takze baza 
    std::cout << "Pojemnosc pojazdu: " << instance.capacity << "\n";

   

    
    //K to zbiór pojazdów np. {1,2,3}
    //|K| to liczebność zbioru  = 3
    //2.1 Constructive heurestic for CCVRP
    //Algorithm 1 
    // 
    // Etap 1: Inicjalizacja
    // dla każdego pojazdu przypisz jednego najbliższego klienta i zaktualizuj dostępną pojemność 
    // usuń przydzielonego klienta  z listy klientów

    std::vector<std::pair<double, int>> distances_from_depot = get_all_distances(instance.depot_id-1, instance);
    std::sort(distances_from_depot.begin(), distances_from_depot.end());
 
   
    std::vector<Route> routes;


    //Zdefiniowanie nowych tras - kazde auto ma po jednej, pustej trasie
    for (int i = 0; i < num_vehicles; ++i) {
        routes.push_back(Route(i, instance.capacity));  // Pojazd i o odpowiedniej pojemności
    }

    //dodajemy po kolei najepsze trasy do kolejnych pojazdow
   // int index = 0;
	std::vector <int> indexes_to_remove;
    for (auto &c : routes)
    {
        auto best = distances_from_depot.front();
        distances_from_depot.erase(distances_from_depot.begin());
       
		//best second to indeks w oryginalnej tablicy instances z wejściowymi danymi
        c.add_customer(instance.nodes[best.second],best.first,1);
		indexes_to_remove.push_back(best.second);
    }

	//usuwanie dodanych klientow z listy wszstkich klientów
    std::sort(indexes_to_remove.begin(), indexes_to_remove.end(), std::greater<int>());
    for (int index : indexes_to_remove) {
        if (index >= 0 && index < instance.nodes.size()) {
            instance.nodes.erase(instance.nodes.begin() + index);
        }
	}
	//zwolnie pamięci
    std::vector<int>().swap(indexes_to_remove);

    //wstawienie pozostalych klientow
	//usuniecie magazynu z listy klientów
    instance.nodes.erase(instance.nodes.begin());

 

   //ETAP 2 
    for (int i = instance.nodes.size() - 1; i >=0; i--)
    {
        InsertionResult best_feasible_insertion;
        InsertionResult best_any_insertion;
        best_feasible_insertion.cost = std::numeric_limits<double>::infinity();
        best_any_insertion.cost = std::numeric_limits<double>::infinity();


		//r-1 ?? - raczej nie bo niby dlaczego - musimy przejsc przez wszystkie pojazdy
        for (int r = 0; r < routes.size(); r++)
        {
            InsertionResult insertion = find_best_insertion_improved(routes[r], instance.nodes[i]);
            // Zapisz najlepszą wstawkę tylko jeśli trasa ma wolne miejsce
            if (instance.nodes[i].demand <= routes[r].remaining_capacity)
            {
                if (insertion.cost < best_feasible_insertion.cost)
                {
                    best_feasible_insertion = insertion;
                }
            }

            // Niezależnie od pojemności – zapisz najlepszą ogólnie
            if (insertion.cost < best_any_insertion.cost)
            {
                best_any_insertion = insertion;
            }
        }

        InsertionResult chosen_insertion = (best_feasible_insertion.cost < std::numeric_limits<double>::infinity())
            ? best_feasible_insertion
            : best_any_insertion;
   
        routes[chosen_insertion.route_id].add_customer(instance.nodes[i], chosen_insertion.cost, chosen_insertion.place,false);
        instance.nodes.erase(instance.nodes.begin() + i);
    }



    std::cout << "Koniec algorithm 1";


    //w routes pierwszy element to zawsze baza, czyli routes[i][0] == 1 


    // Etap 2: Przypisanie pozostałych klientów
    // weź pierwszego wolnego klienta i sprawdź dla wszystkich dostępnych pojazdów (tych co mają wolne miejsce)
    // wszystkie pozycje i wstaw go gdzie wartość delty jest minimalna
    //          ogólnie trzeba sprawdzić wszystkie pozycjie dla wszystkich pojazdów
    // jeżeli NIE MA pojazdu który pomieści wstaw go poprostu do najlepszej trasy, nie patrząc na ładowność - ale tylko i wylacznie 
    //gdy nie ma juz wolnego miejsca 
    //delta jest przyblizona wartoscia kosztu wstawienia nowego klienta, a nie dokludnym przyrostem wzgledem 'oryginalu'
    // zaktualizuj pojemność pojazdu po wstawieniu 
    //powtórz dla każdego celu 

    //DELTA
        

    for (int i = 0; i < routes.size(); i++)
    {
   	routes[i].route_cost = g(routes[i]); // oblicz koszt trasy
    }


	//Etap 3: VND
   //  routes = VND(routes);
   for (int i = 0; i < routes.size(); i++)
   {
       routes[i].route_cost_improved = g(routes[i]); // oblicz koszt trasy
   }
     std::cout << "START P =-=--===-";

     std::vector<Route> routess =  P(routes, 2);
     std::cout << "KONIEC P =-=--===-";
}

