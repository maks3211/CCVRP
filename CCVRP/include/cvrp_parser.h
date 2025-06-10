#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include "cvrp_types.h"

CVRPInstance parseCVRPFile(const std::string& filename);