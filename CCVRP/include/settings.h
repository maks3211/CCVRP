#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include "skewed_vns.h"
#include "hybridAvnsLns.h"
#include "bso.h"
#include "../src/json.hpp"


using json = nlohmann::json;
class Settings {
public:
    Settings();
    int number_of_starts_hybrid;
    int number_of_starts_skewed;
    int number_of_starts_bso;

    int num_vehicles;

    brainConfig bso;
    hybridAvnsLnsConfig hybrid;
    SkewedVNSConfig skewed;

    std::string instance_name;
    std::string result_folder_name;
    std::string input_path;
    std::string main_result_path;
    std::string folder_name;

    bool load_from_file(const std::string& file_name);
  
};