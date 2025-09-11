#include "utils.h"

double euclidean_distance(double x1, double y1, double x2, double y2)
{
    
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}
double euclidean_distance(Node n1, Node n2)
{

    double res = sqrt((n1.x - n2.x) * (n1.x - n2.x) + (n1.y - n2.y) * (n1.y - n2.y));
    return res;
}

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


InsertionResult find_best_insertion(Route& route, Node& i)
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

        b = euclidean_distance(route.customers[j], i);
        //ZASTAPIENIE TEGO CO JEST POD AKTUALNYM TMP, I DODANIE MNOZENIE 2*a - W WYZNACZANIU CURRENTCOST
        int tmp = p - j;
        //int tmp = p - j;
       // if (tmp <= 0)
        //{
        //    c = 0;
       // }
      //  else if (tmp > 0)
      // {
            c = tmp * (euclidean_distance(route.customers[j], i) + euclidean_distance(route.customers[j + 1], i));
       // }

        currentCost = 2*a + b + c;
        if (currentCost < cost)
        {
            cost = currentCost;
            place = j;
        }
    }
    return InsertionResult(place, route.vehicle_id, cost);  // place - po ktorym wstawic nowego klienta if place == 0 to nowy bedzie na idx 1 
}


double g(Route& pi)
{
    int p = pi.customers.size() - 1; // liczba klientów w trasie (minus baza)
    double result = 0.0;
    for (int i = 1; i <= p; i++)
    {
        result += (p - i + 1) * euclidean_distance(pi.customers[i - 1], pi.customers[i]);
    }
    return result;
}

void calculate_cost(Result& result)
{
    double total_cost = 0.0;
    double cost;
    for (auto& route : result.routes)
    {
        cost = g(route);
		route.route_cost = cost;
        total_cost += cost;
    }
	result.total_cost = total_cost;
}

void calculate_remaining_capacity(Result& result)
{
	
    for (auto& route : result.routes)
    {
        int total_demand = 0;
        for (auto& node : route.customers)
        {
            total_demand += node.demand;
        }
        route.remaining_capacity = route.initial_capacity - total_demand;
    }
}

int dif(std::vector<Route>& s1, std::vector<Route>& s2)
{

    std::unordered_map<int, int> successors_s1;
    std::unordered_map<int, int> successors_s2;

    for (const auto& route : s1)
    {
        for (size_t i = 0; i < route.customers.size() - 1; ++i) {
            int current_id = route.customers[i].id;
            int next_id = route.customers[i + 1].id;
            successors_s1[current_id] = next_id;
        }
    }

    for (const auto& route : s2)
    {
        for (size_t i = 0; i < route.customers.size() - 1; ++i) {
            int current_id = route.customers[i].id;
            int next_id = route.customers[i + 1].id;
            successors_s2[current_id] = next_id;
        }
    }
    int difference = 0;


    for (const auto& pair : successors_s1) {
        int client_id = pair.first;
        int succ_id_s1 = pair.second;
        auto it = successors_s2.find(client_id);
        if (it == successors_s2.end() || it->second != succ_id_s1) {
            ++difference;
        }
    }
    return difference;
}

bool has_duplicate_ids_in_route(const std::vector<Node>& customers) {
    std::unordered_set<int> ids_seen;
    for (const auto& customer : customers) {
        if (ids_seen.count(customer.id) > 0) {
            std::cout << "Duplicate found: " << customer.id << "\n";
            return true;  // duplikat znaleziony
        }
        ids_seen.insert(customer.id);
    }

    return false;  // brak duplikatów
}

bool any_duplicates(const std::vector<Route>& routes) {
    for (const auto& route : routes) {
        if (has_duplicate_ids_in_route(route.customers)) {
            return true;
        }
    }
    return false;
}

bool any_global_duplicates(const std::vector<Route>& routes) {
    std::unordered_map<int, std::vector<int>> client_to_routes;

    for (size_t route_index = 0; route_index < routes.size(); ++route_index) {
        const auto& customers = routes[route_index].customers;

        for (size_t i = 1; i < customers.size(); ++i) { // Pomijamy punkt startowy
            int id = customers[i].id;
            client_to_routes[id].push_back(static_cast<int>(route_index));
        }
    }

    bool any_duplicates = false;
    for (const auto& pair : client_to_routes) {
        const int id = pair.first;
        const std::vector<int>& routes_with_client = pair.second;

        if (routes_with_client.size() > 1) {
            any_duplicates = true;
            std::cout << "❗ Client ID " << id << " appears in routes: ";
            for (int route_idx : routes_with_client) {
                std::cout << route_idx << " ";
            }
            std::cout << "\n";
        }
    }
    return any_duplicates;
}

bool has_negtive_capacity(const Result& result)
{
    for(auto &a : result.routes)
    {
        if (a.remaining_capacity < 0)
        {
			return true;
        }
    }
	return false;
}
