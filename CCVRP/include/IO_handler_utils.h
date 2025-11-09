#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <regex>
#include <iomanip>
#include <iostream>
#include <filesystem>
#include "cvrp_types.h"
#include "route.h"

class IO_handler_utils {
public:
	static std::string create_table(Result& solution);
	static void draw_horizontal_lines(std::ostringstream& oss, int width);
};