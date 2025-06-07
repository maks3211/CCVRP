#include "ResultsParser.h"

std::vector<Route> load_routes_from_file(
    const std::string& filename,
    const CVRPInstance& instance
) {
    // Tworzymy mapę id → Node
    std::unordered_map<int, Node> node_map;
    for (const auto& node : instance.nodes) {
        node_map[node.id] = node;
    }

    std::ifstream file(filename);
    std::string line;
    std::vector<Route> routes;
    int vehicle_id = 1;

    while (std::getline(file, line)) {
        if (line.empty() || line.find("Route") == std::string::npos)
            continue;

        size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos)
            continue;

        std::istringstream iss(line.substr(colon_pos + 1));
        std::vector<int> ids;
        int id;
        while (iss >> id) {
            ids.push_back(id);
        }

        if (ids.empty())
            continue;

        // Pomijamy końcowe "1" (depot) jeśli jest na końcu
        if (ids.back() == instance.depot_id) {
            ids.pop_back();
        }

        Route route(vehicle_id++, instance.capacity);

        for (int client_id : ids) {
            if (client_id == instance.depot_id)
                continue;

            auto it = node_map.find(client_id);
            if (it != node_map.end()) {
                const Node& client = it->second;
                route.customers.push_back(client);
                route.remaining_capacity -= client.demand;
            }
            else {
                std::cerr << "Brak klienta o ID: " << client_id << " w danych wejściowych.\n";
            }
        }

        routes.push_back(route);
    }

    return routes;
}