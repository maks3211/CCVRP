#include "random_utils.h"


 std::mt19937 rng(std::random_device{}());

std::pair<int, int> get_random_client(std::vector<Route>& s) 
{

    int random_route;
    do {
        std::uniform_int_distribution<> route_dist(0, s.size() - 1);
        random_route = route_dist(rng);
    } while (s[random_route].customers.size() <= 1);
    
    std::uniform_int_distribution<> client_dist(1, s[random_route].customers.size() - 1);
    int random_client = client_dist(rng);
    return { random_route, random_client };
}


//reurns first index of clients group , -1 if not possible
int get_random_clients_from_route(Route& route, int num_clients) 
{
	int number_of_clients = route.customers.size() - 1; // pomijamy depozyt
    if (number_of_clients <= 1 || num_clients > number_of_clients) {
		return -1; // pusta trasa, lub za duzo klientow do wylosowania
    }

    int min = 1;
	int max = number_of_clients - num_clients + 1; //max index klienta w trasie 
    std::uniform_int_distribution<> dist(min, max);
    return dist(rng);
    

}



std::vector<int> get_random_route_indexes(std::vector<Route>& s)
{
	int size = s.size();
    std::vector<int> indices(size);
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(), rng);
	return indices;
}