#pragma once

#include <windows.h>
#include <iostream>
#include "cvrp_types.h"

#include "IO_handlerV2.h"
#include "hybridAvnsLns.h"
#include "bso.h"
#include "skewed_vns.h"
class Ui
{
public:

	Ui(brainConfig& bso_config, hybridAvnsLnsConfig& hybrid_config, SkewedVNSConfig& sekwed_config,
		int& number_bso, int& number_hybrid, int& number_skewed,
		std::string& input_path, std::string& instance_name,int& num_vehicles,
		std::string& result_path, std::string& result_folder);


	brainConfig& bso_config;
	hybridAvnsLnsConfig& hybrid_config;
	SkewedVNSConfig& sekwed_config;
	int& number_bso;
	int& number_hybrid;
	int& number_skewed;

	std::string& input_path;
	std::string& instance_name; 
	int& num_vehicles;

	std::string& result_path;
	std::string& result_folder;


	void main_menu();
	void set_params();
	void set_bso_params(brainConfig& config);
	void set_hybrid_params(hybridAvnsLnsConfig& config);
	void set_skewed_params(SkewedVNSConfig& config);

	void set_input_path();
	void set_full_path_input();
	void set_instance_name();

	void set_result_path();

	void set_number_of_runs();

	void print_type_in_value_for(std::string param);

	std::string get_line_with_default(const std::string& prompt, const std::string& defaultValue);
};


