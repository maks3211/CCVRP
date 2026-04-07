#pragma once
#include <iostream>
#include <unordered_set>
#include <unordered_map>

#include "cvrp_types.h"
#include "route.h"
#define M_PI       3.14159265358979323846
/// <summary>
/// Calculates the Euclidean distance between two 2D points
/// </summary>
/// <param name="x1">X coordinate of the first point</param>
/// <param name="y1">Y coordinate of the first point</param>
/// <param name="x2">X coordinate of the second point</param>
/// <param name="y2">Y coordinate of the second point</param>
/// <returns>The Euclidean distance between the two points</returns>
double euclidean_distance(double x1, double y1, double x2, double y2);


/// <summary>
/// Calculates the Euclidean distance between two nodes
/// </summary>
/// <param name="n1">- First node</param>
/// <param name="n2">- Second node</param>
/// <returns>The Euclidean distance between the two nodes</returns>
double euclidean_distance(Node n1, Node n2);

/// <summary>
/// Simple struct to hold distance and node id - node id is the id from input file - not index
/// </summary>
struct DistanceInfo {
    double distance;
    int node_id;
    bool operator<(const DistanceInfo& other) const {
        return distance < other.distance;
	}
};



/// <summary>
/// Computes distances from a given node to all other nodes in the CVRP instance
/// </summary>
/// <param name="from_node">- The index of the node from which distances are calculated</param>
/// <param name="instance">- The CVRPInstance containing all nodes</param>
/// <returns>A vector of pairs representing distances and node indices</returns>
std::vector<DistanceInfo> get_all_distances(const int from_node, const CVRPInstance& instance);


InsertionResult find_best_insertion(Route& route, Node& i);

/// <summary>
/// Koszt calej trasy
/// </summary>
/// <param name="pi"></param>
/// <returns></returns>
double g(Route& pi);
double g(std::vector<Node>& pi);


void calculate_distance(Result& result);

void calculate_cost(Result &result);
double calculate_cost(std::vector<Route>& routes);
int calculate_remaining_capacity(Result& result);
int calculate_remaining_capacity(Route& route);
int calculate_remaining_capacity(std::vector<Route>& routes);

int calculate_used_capacity(std::vector<Node>& route);

bool add_customer_at_index_with_penalty(Route& route,Node& client);
InsertionResult calculate_insertion_cost(Route& route, Node& i, int insertion_index);

int get_remaining_capacity(const Route& route);
int get_total_remaining_capacity(const std::vector<Route>& routes);
int get_total_remaining_capacity(const Result& result);

int dif(std::vector<Route>& s1, std::vector<Route>& s2);

/// <summary>
/// Checks for duplicate customer IDs within a single route
/// </summary>
/// <param name="customers">- A vector of Node objects representing customers in a route</param>
/// <returns>true if any duplicate IDs are found; false otherwise</returns>
bool has_duplicate_ids_in_route(const std::vector<Node>& customers);


/// <summary>
/// Checks all routes for internal duplicates
/// </summary>
/// <param name="routes">- A vector of Route objects</param>
/// <returns>true if at least one route contains duplicates; false otherwise</returns>
bool any_duplicates(const std::vector<Route>& routes);


/// <summary>
///  Checks for global duplicates across multiple routes.
/// </summary>
/// <param name="routes">- A vector of Route objects</param>
/// <returns>true if any customer is assigned to more than one route; false otherwise</returns>
bool any_global_duplicates(const std::vector<Route>& routes);


bool has_negtive_capacity(const Result& result);

/// <summary>
/// Returns the index of the route with the smallest violation of capacity constraints.
/// 
/// </summary>
/// <param name="routes"></param>
/// <returns>Index of best route</returns>
int find_route_with_smallest_violation(const std::vector<Route>& routes);


//Funkcje utworzone na potrzeby metody hybrid - 198 - wykorzystywane do obliczania gain - umieszczone w tym miejcu ze wzgledu na to ze moze w przyszlosci beda wykorzystane tez do brain storm opt

//wyznacza kat miedzy dwoma punktami w stopniach
double get_angle_deg(const Node& depot, const Node& client);


