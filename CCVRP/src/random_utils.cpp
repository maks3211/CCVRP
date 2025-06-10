#include "random_utils.h"


std::pair<int, int> get_random_client(std::vector<Route>& s) 
{
    std::random_device rd;
    std::mt19937 gen(rd());
    
    int random_route;
    do {
        std::uniform_int_distribution<> route_dist(0, s.size() - 1);
        random_route = route_dist(gen);
    } while (s[random_route].customers.size() <= 1);
    
    std::uniform_int_distribution<> client_dist(1, s[random_route].customers.size() - 1);
    int random_client = client_dist(gen);
    return { random_route, random_client };
}