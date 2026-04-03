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

std::vector<DistanceInfo> get_all_distances(const int from_node, const CVRPInstance& instance) // zwraca wektor odległości węzła from_node do pozstałych punktów
{
    if (from_node >= instance.dimension)
    {
        return{};
    }
    std::vector<DistanceInfo> distances;
    for (int i = 0; i < instance.dimension; i++)
    {
        if (i == from_node)
        {
            continue;
        }
        else
        {
          //  distances.push_back({ euclidean_distance(instance.nodes[from_node], instance.nodes[i]), i });
           distances.push_back({ euclidean_distance(instance.nodes[from_node], instance.nodes[i]), instance.nodes[i].id});
        }
    }
    return distances;
}


InsertionResult find_best_insertion(Route& route, Node& i)
{
  
    int place = 1;
    double cost = std::numeric_limits<double>::infinity();
    int p = route.customers.size() - 1;
	bool feasible = false;  
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
        int tmp = p - j + 1;
        if (p - j + 1 < 1)
        {
            std::cout << "UWAGA JEST PONIZEJ JEDEN";
        }
        //int tmp = p - j;
       // if (tmp <= 0)
        //{
        //    c = 0;
       // }
      //  else if (tmp > 0)
      // {
            //przypadek dodania nowego klinta na koniec trasy 
            if (j != p)
            {
                c += euclidean_distance(route.customers[j + 1], i);
            }
            c += euclidean_distance(route.customers[j], i);
            c *= tmp;


       // }

        currentCost = 2*a + b + c;
      
       // double currentCost = calculate_insertion_cost(route,i,j).cost;
        
        if (currentCost < cost)
        {
            cost = currentCost;
            place = j;
            if (route.remaining_capacity - i.demand >= 0)
            {
                feasible = true;
            }
            else
            {
				feasible = false;
            }
        }
    }
    return InsertionResult(place, route.vehicle_id, cost,i.id, feasible);  // place - po ktorym wstawic nowego klienta if place == 0 to nowy bedzie na idx 1 
}



InsertionResult calculate_insertion_cost(Route& route, Node& i, int insertion_index)
{
    InsertionResult res;
    res.place = insertion_index;
    res.route_id = route.vehicle_id;
    double currentCost = 0.0;
    double a = 0.0;
    double b = 0.0;
    double c = 0.0;
    int p = route.customers.size() - 1;

    for (int h = 0; h < insertion_index; h++)
    {
        a += euclidean_distance(route.customers[h], route.customers[h + 1]);
    }

    b = euclidean_distance(route.customers[insertion_index], i);
    //ZASTAPIENIE TEGO CO JEST POD AKTUALNYM TMP, I DODANIE MNOZENIE 2*a - W WYZNACZANIU CURRENTCOST
    int tmp = p - insertion_index + 1;
    if (p - insertion_index + 1 < 1)
    {
        std::cout << "UWAGA JEST PONIZEJ JEDEN";
    }
    //int tmp = p - j;
   // if (tmp <= 0)
    //{
    //    c = 0;
   // }
  //  else if (tmp > 0)
  // {
        //przypadek dodania nowego klinta na koniec trasy 
    if (insertion_index != p)
    {
        c += euclidean_distance(route.customers[insertion_index + 1], i);
    }
    c += euclidean_distance(route.customers[insertion_index], i);
    c *= tmp;


    // }
    res.feasible = (i.demand <= route.remaining_capacity);
	res.cost = 2 * a + b + c;
    return res;
}




double g(Route& pi)
{
    int p = pi.customers.size() - 1; // liczba klientów w trasie (minus baza)
    double result = 0.0;
    pi.cumulative_costs[0] = 0.0;
    for (int i = 1; i <= p; i++)
    {
        result += (p - i + 1) * euclidean_distance(pi.customers[i - 1], pi.customers[i]);
		pi.cumulative_costs[i] = result; // zapis kosztu skumulowanego do klienta i
    }
    //to dodalem!!!
	pi.route_cost = result;
    return result;
}



double g(std::vector<Node>& pi)
{
    int p = pi.size() - 1; // liczba klientów w trasie (minus baza)
    double result = 0.0;
   
    for (int i = 1; i <= p; i++)
    {
        result += (p - i + 1) * euclidean_distance(pi[i - 1], pi[i]);
       
    }
    return result;
}
//liczy poknany dystans pojazdow 
void calculate_distance(Result& result)
{
    double total_cost = 0.0;

    for (auto& route : result.routes)
    {
        double route_cost = 0.0;

        for (size_t i = 0; i < route.customers.size() - 1; ++i)
        {
            route_cost += euclidean_distance(route.customers[i], route.customers[i + 1]);
        }

        route.route_cost = route_cost; // zapis kosztu trasy
        total_cost += route_cost;      // sumowanie do kosztu całego rozwiązania
    }

    result.total_cost = total_cost;
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


double calculate_cost(std::vector<Route>& routes)
{
    double total_cost = 0.0;
    double cost;
    for (auto& route : routes)
    {
        cost = g(route);
        route.route_cost = cost;
        total_cost += cost;
    }
	return total_cost;
}

int calculate_remaining_capacity(Result& result)
{
    int total_remaining = 0;
    for (auto& route : result.routes)
    {
        int total_demand = 0;
        for (auto& node : route.customers)
        {
            total_demand += node.demand;
        }
        route.remaining_capacity = route.initial_capacity - total_demand;
        total_remaining += route.remaining_capacity;
    }
    return total_remaining;
}

int calculate_remaining_capacity(Route& route)
{
    int total_demand = 0;
    for (auto& node : route.customers)
    {
        total_demand += node.demand;
    }
    route.remaining_capacity = route.initial_capacity - total_demand;
    return route.remaining_capacity;
}

int calculate_remaining_capacity(std::vector<Route>& routes)
{
    int total_remaining = 0;
    for (auto& route : routes)
    {
        int total_demand = 0;
        for (auto& node : route.customers)
        {
            total_demand += node.demand;
        }
        route.remaining_capacity = route.initial_capacity - total_demand;
        total_remaining += route.remaining_capacity;
    }
    return total_remaining;
}




int calculate_used_capacity(std::vector<Node>& route)
{
	int total_demand = 0;
    for (int i = 0; i < route.size(); i++)
    {
		total_demand += route[i].demand;
    }
	return total_demand;
}



bool add_customer_at_index_with_penalty(Route& route, Node& client)
{
    InsertionResult insert = find_best_insertion(route, client);
    route.customers.insert(route.customers.begin() + insert.place, client);
    route.route_cost += insert.cost;
    route.remaining_capacity -= client.demand;
    route.is_penalized = true;
    //CZY MA BYC 50?? OD CZEGO TO ZALEZY
    route.penatly_eta += 50.0;
    return true;
}


int get_total_remaining_capacity(const Result& result)
{
    int total_remaining = 0;
    for (const auto& route : result.routes)
    {
        int total_demand = 0;
        for (const auto& node : route.customers)
        {
            total_demand += node.demand;
        }
        total_remaining += (route.initial_capacity - total_demand);
    }
    return total_remaining;
}

// Dla pojedynczej trasy
int get_remaining_capacity(const Route& route)
{
    int total_demand = 0;
    for (const auto& node : route.customers)
    {
        total_demand += node.demand;
    }
    return route.initial_capacity - total_demand;
}

// Dla wektora tras
int get_total_remaining_capacity(const std::vector<Route>& routes)
{
    int total_remaining = 0;
    for (const auto& route : routes)
    {
        int total_demand = 0;
        for (const auto& node : route.customers)
        {
            total_demand += node.demand;
        }
        total_remaining += (route.initial_capacity - total_demand);
    }
    return total_remaining;
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
            std::cout << "Client ID " << id << " appears in routes : ";
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



int find_route_with_smallest_violation(const std::vector<Route>& routes)
{
    int best_route = 0;
    double min_violation = std::numeric_limits<double>::infinity();

    for (size_t r = 0; r < routes.size(); ++r)
    {
        int violation = get_remaining_capacity(routes[r]);
        if (violation < min_violation)
        {
            min_violation = violation;
            best_route = static_cast<int>(r);
        }
    }
    return best_route;
}
