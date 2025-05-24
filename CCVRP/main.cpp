

#include <iostream>
#include <algorithm>
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
   // std::cout << " start ";
    for (int j = 0; j < route.customers.size(); j++)
    {
        double currentCost = 0.0;
        double a = 0.0;
        double b = 0.0;
        double c = 0.0;
       // std::cout << " jeen  ";
        for (int h = 0; h < j; h++)
        {
           // std::cout << " przeddwa  ";
			a += euclidean_distance(route.customers[h], route.customers[h + 1]);
            //std::cout << " dwa  ";
        }

        b = euclidean_distance(route.customers[j],i);
      //  std::cout << " trzy  ";
        int tmp = p - j;
        if (tmp <= 0)
        {
           // std::cout << " zero ";
            c = 0;
        }
        else if (tmp > 0)
        {    
           // std::cout << " przedcztery  ";
            c = tmp * (euclidean_distance(route.customers[j],i) + euclidean_distance(route.customers[j+1], i));
           // std::cout << " cztery  ";
        }

		currentCost = a + b + c;    
        if (currentCost < cost)
        {
            cost = currentCost;
            place = j;
        }
    }
   // std::cout << " return  " << std::endl;
	return InsertionResult(place, route.vehicle_id, cost);  // place - po ktorym wstawic nowego klienta if place == 0 to nowy bedzie na idx 1 
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
    for (int i = 0; i <= num_vehicles; ++i) {
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
	//usuniecie bazy z listy klientów
    instance.nodes.erase(instance.nodes.begin());

 

    //od tego zaczac
    for (int i = instance.nodes.size() - 1; i >=0; i--)
    {
        InsertionResult best_insertion;
        best_insertion.cost = std::numeric_limits<double>::infinity();


        //r-1 ??
        for (int r = 0; r < routes.size(); r++)
        {
            if(instance.nodes[i].demand > routes[r].remaining_capacity)
            {
                //std::cout << "ZAPOTRZEBOWANIE: " << instance.nodes[i].demand <<" ile miejsca " << routes[r].remaining_capacity << std::endl;
                continue; // nie ma miejsca w trasie
		    }
        
            InsertionResult insertion = find_best_insertion_improved(routes[r], instance.nodes[i]);
   
            if (insertion.cost < best_insertion.cost )
            {
              best_insertion = insertion;
            }
        }

        //sprawdzono wszystkie pojazdy dla danego klienta
        //TODO dodac warunek jezeli nie bedzie zadanych dostepnch miejsc to force 
		routes[best_insertion.route_id].add_customer(instance.nodes[i], best_insertion.cost, best_insertion.place);
        instance.nodes.erase(instance.nodes.begin() + i);
        best_insertion.cost = std::numeric_limits<double>::infinity();
    }






    //najlepsze wstawienie klienta 1 do trasu 0
    std::cout << "Koniec";

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
        






}

