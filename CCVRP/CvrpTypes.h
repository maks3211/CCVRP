#ifndef CVRP_TYPES_HPP
#define CVRP_TYPES_HPP

#include <vector>
#include <string>

struct Node {
    int id;
    double x, y;
    int demand = 0;
};

struct CVRPInstance {
    std::string name;
    int dimension = 0;
    int capacity = 0;
    std::vector<Node> nodes;
    int depot_id = -1;
};

#endif // CVRP_TYPES_HPP