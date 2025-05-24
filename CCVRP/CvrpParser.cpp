#include "CvrpParser.h"
#include <fstream>
#include <sstream>

CVRPInstance parseCVRPFile(const std::string& filename) {

    CVRPInstance instance;
    std::ifstream file(filename);
    std::string line;

    enum Section { NONE, NODE_COORD, DEMAND, DEPOT };
    Section current = NONE;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key;
        iss >> key;

        if (key == "NAME") {
            iss.ignore(3);
            std::getline(iss, instance.name);
        }
        else if (key == "DIMENSION") {
            iss.ignore(3);
            iss >> instance.dimension;
            instance.nodes.resize(instance.dimension);
        }
        else if (key == "CAPACITY") {
            iss.ignore(3);
            iss >> instance.capacity;
        }
        else if (key == "NODE_COORD_SECTION") {
            current = NODE_COORD;
        }
        else if (key == "DEMAND_SECTION") {
            current = DEMAND;
        }
        else if (key == "DEPOT_SECTION") {
            current = DEPOT;
        }
        else if (key == "EOF") {
            break;
        }
        else {
            if (current == NODE_COORD) {
                int id;
                double x, y;
                std::istringstream(line) >> id >> x >> y;
                instance.nodes[id - 1].id = id;
                instance.nodes[id - 1].x = x;
                instance.nodes[id - 1].y = y;
            }
            else if (current == DEMAND) {
                int id, demand;
                std::istringstream(line) >> id >> demand;
                instance.nodes[id - 1].demand = demand;
            }
            else if (current == DEPOT) {
                int depot;
                std::istringstream(line) >> depot;
                if (depot != -1)
                    instance.depot_id = depot;
            }
        }
    }
    return instance;
}
