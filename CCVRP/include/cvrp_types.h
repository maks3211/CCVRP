#pragma once
#include <vector>
#include <string>

struct Node {
    int id;
    double x, y;
    int demand = 0;
    Node() = default;
    Node(int i, double x, double y, int d) : id(i), x(x), y(y), demand(d) {}
};

struct CVRPInstance {
    std::string name;
    int dimension = 0;
    int capacity = 0;
    std::vector<Node> nodes;
    int depot_id = -1;
};

struct InsertionResult
{
    int place;
    int route_id;
    double cost;
    int client_index = -1; 
	bool feasible = false; //true if insertion is feasible (does not violate capacity constraint)
    InsertionResult() = default;
    InsertionResult(int p, int id, double c) : place(p), route_id(id), cost(c) {}
    InsertionResult(int p, int id, double c, int ci) : place(p), route_id(id), cost(c), client_index(ci) {}
    InsertionResult(int p, int id, double c, int ci, bool fes) : place(p), route_id(id), cost(c), client_index(ci), feasible(fes) {}
};



