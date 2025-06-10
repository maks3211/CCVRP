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


class IO_handler {
	std::string base_name;
	std::string input_path = "inputData/";
	std::string result_path = "Results/";
	CVRPInstance loaded_instance;
	int result_counter = 0;
	void load_instance();
	std::string create_table(Result& solution);
	void draw_horizontal_lines(std::ostringstream& oss, int width);
	void create_folder_if_not_exists(const std::string& folder);
	void move_file_to_folder(const std::string& file_path, const std::string& folder);
	std::string get_new_filename(const std::string& base_name, const std::string& ext, const std::string& folder);
	bool file_exists(const std::string& file_name);
	bool folder_exists(const std::string& folder_name);
public:
	IO_handler(const std::string& base_name);
	CVRPInstance get_instance();	
	void save_solution(Result &solution);
	Result load_solution(int number = 0,std::string name = "");
	void set_base_name(const std::string& new_base_name);
};
