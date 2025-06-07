#pragma once
#include <vector>
#include <string>
#include "Route.h"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <iostream>
std::vector<Route> load_routes_from_file(
    const std::string& filename,
    const CVRPInstance& instance
);
