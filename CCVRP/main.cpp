

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
    return sqrt((n1.x - n2.x) * (n1.x - n2.x) + (n1.y - n2.y) * (n1.y - n2.y));
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


int main()
{

    //instance.nodes 

    std::cout << "Hello World!\n";
    CVRPInstance instance = parseCVRPFile("InputData/Golden_1.vrp");
    std::cout << "Nazwa instancji: " << instance.name << "\n";

    std::cout << "Liczba klientow: " << instance.dimension << "\n"; // w tym zawiera się takze baza 
    std::cout << "Pojemnosc pojazdu: " << instance.capacity << "\n";

    std::cout << "Node 1 " << instance.nodes[1].x;

    
    //K to zbiór pojazdów np. {1,2,3}
    //|K| to liczebność zbioru  = 3
    //2.1 Constructive heurestic for CCVRP
    //Algorithm 1 
    // 
    // Etap 1: Inicjalizacja
    // dla każdego pojazdu przypisz jednego najbliższego klienta i zaktualizuj dostępną pojemność 
    // usuń przydzielonego klienta  z listy klientów

    std::vector<std::pair<double, int>> distances_from_depot = get_all_distances(instance.depot_id, instance);
    std::sort(distances_from_depot.begin(), distances_from_depot.end());
 
    const int num_vehicles = 9;
    std::vector<Route> routes;


    //Zdefiniowanie nowych tras - kazde auto ma po jednej, pustej trasie
    for (int i = 1; i <= num_vehicles; ++i) {
        routes.push_back(Route(i, instance.capacity));  // Pojazd i o odpowiedniej pojemności
    }

    //dodajemy po kolei najepsze trasy do kolejnych pojazdow
   // int index = 0;
    std::cout << "asd";
    for (auto &c : routes)
    {
        auto best = distances_from_depot.front();
        distances_from_depot.erase(distances_from_depot.begin());
       
		//best second to indeks w oryginalnej tablicy instances z wejściowymi danymi
        c.add_customer(instance.nodes[best.second],best.first);

        //tutaj tez usunac klienta z instance.nodes  second - to powinien byc indeks z instance.nodes[best.second]
        //raczej trzeba zapisywać tylko indeksy ktore usunac, posortowac malejaco i usunac poza petla
//        int index = best.second;
//        if (index >= 0 && index < instance.nodes.size()) {
//            instance.nodes.erase(instance.nodes.begin() + index);
//        }
     
    }
    auto best = distances_from_depot.front();
    distances_from_depot.erase(distances_from_depot.begin());
    routes[0].add_customer(instance.nodes[best.second], best.first);
    std::cout << "asd";

    

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

