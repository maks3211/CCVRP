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
    InsertionResult() = default;
    InsertionResult(int p, int id, double c) : place(p), route_id(id), cost(c) {}
};



