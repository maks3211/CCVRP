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
#include "IO_handler_utils.h"
#include "../src/json.hpp"


namespace IO_handlerV2
{
	class IO_handler {
		std::string input_path = "";
		std::string result_path = "";
		void load_instance();
		CVRPInstance loaded_instance;

	public:
		CVRPInstance get_instance();
		void save_solution(Result& solution, std::string add_to_name = "",std::string additional_info = "");
		void save_progress(Result& solution);
		bool set_input_path(const std::string& path);
		bool set_result_path(const std::string& path);
		bool save_progress_enabled = true;
	};
}
