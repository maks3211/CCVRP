#include "random_utils.h"

//MOZNA TO ZOPTYMALIZOWAC NP TAK ABY KORZYSTAC ZE WSPOLENGO UNIFORM_INT_DISTRUBUTION ITP!!!
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


std::vector<std::pair<int, int>> get_n_random_clients(std::vector<Route>& s, int n) {
    std::vector<std::pair<int, int>> all_clients;

    
    for (int r = 0; r < s.size(); ++r) {
        for (int c = 1; c < s[r].customers.size(); ++c) {
            all_clients.emplace_back(r, c);
        }
    }

    if (all_clients.size() < static_cast<size_t>(n)) {
        return {};  // Za ma³o klientów
    }

   
    std::shuffle(all_clients.begin(), all_clients.end(), rng);
    all_clients.resize(n);

    return all_clients;
}


std::vector<std::pair<int, int>> get_n_random_clients_diff_routes(
    std::vector<Route>& s, int n, int min_index, bool exclude_last)
{
    std::vector<std::pair<int, int>> result;

    int num_routes = s.size();
    if (num_routes == 0) return result;

    int k = std::min(n, num_routes);

    std::vector<int> route_indices(num_routes);
    for (int i = 0; i < num_routes; ++i)
        route_indices[i] = i;

    std::shuffle(route_indices.begin(), route_indices.end(), rng);

    for (int i = 0; i < k; ++i) {
        int r = route_indices[i];
        int size = s[r].customers.size();

        
        int start = min_index;
        int end = exclude_last ? size - 2 : size - 1;

       
        if (start > end)
            continue;

        std::uniform_int_distribution<int> dist(start, end);
        int c = dist(rng);

        result.emplace_back(r, c);
    }

    return result;
}


//[0,1)
double random_01() {
    static std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng);  
}

//[from, to] - moze wylosowac from jak i to
int random_int_from_to(int from, int to)
{
    std::uniform_int_distribution<int> dist(from, to);  
    int liczba = dist(rng);
    return liczba;
}