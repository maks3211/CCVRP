#include "IO_handlerV2.h"

namespace IO_handlerV2
{
    bool IO_handler::set_input_path(const std::string& path)
    {
        if (std::filesystem::exists(path)) {
            input_path = path;
            return true;
        }
        return false;
	}

   /* bool IO_handler::set_result_path(const std::string& path)
    {
        if (std::filesystem::exists(path)) {
            result_path = path;
            return true;
        }
        return false;
	}
    */

    bool IO_handler::set_result_path(const std::string& path)
    {
        std::filesystem::path p(path);

        if (!std::filesystem::exists(p)) {
            // utwórz katalog (równie¿ poœrednie)
            if (!std::filesystem::create_directories(p)) {
                return false; // nie uda³o siê utworzyæ
            }
        }

        result_path = path;
        return true;
    }

	CVRPInstance IO_handler::get_instance()
	{
		if (loaded_instance.nodes.empty()) {
			IO_handler::load_instance();
		}
		return loaded_instance;
	}


    void IO_handler::load_instance()
    {

        std::ifstream file(input_path);
        std::string line;

        enum Section { NONE, NODE_COORD, DEMAND, DEPOT };
        Section current = NONE;

        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string key;
            iss >> key;

            if (key == "NAME") {
                iss.ignore(3);
                std::getline(iss, loaded_instance.name);
            }
            else if (key == "DIMENSION") {
                iss.ignore(3);
                iss >> loaded_instance.dimension;
                loaded_instance.nodes.resize(loaded_instance.dimension);
            }
            else if (key == "CAPACITY") {
                iss.ignore(3);
                iss >> loaded_instance.capacity;
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
                    loaded_instance.nodes[id - 1].id = id;
                    loaded_instance.nodes[id - 1].x = x;
                    loaded_instance.nodes[id - 1].y = y;
                }
                else if (current == DEMAND) {
                    int id, demand;
                    std::istringstream(line) >> id >> demand;
                    loaded_instance.nodes[id - 1].demand = demand;
                }
                else if (current == DEPOT) {
                    int depot;
                    std::istringstream(line) >> depot;
                    if (depot != -1)
                        loaded_instance.depot_id = depot;
                }
            }
        }

    }

    //zapisuje rozwiazanie do wybranego folderu - nie tworzy samemu folderu
    void IO_handler::save_solution(Result& solution, std::string add_to_name,std::string additional_info, const std::vector<double>& run_results)
    {
        std::filesystem::path folder(result_path);

        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);

        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &now_time);
#else
        localtime_r(&now_time, &tm);
#endif

        std::ostringstream ss;
        ss << std::put_time(&tm, "%d_%m_%H_%M_%S");

        std::string filename = add_to_name + "_" + ss.str() + ".vrp";
        std::filesystem::path file = folder / filename;

        std::filesystem::create_directories(folder);

        std::ofstream out(file);  
        if (!out.is_open())
        {
            throw std::runtime_error("Nie mozna otworzyc pliku do zapisu: " + file.string());
        }

        for (int i = 0; i < solution.routes.size(); i++)
        {

            out << "Route #" << (i) << ": ";
            for (const auto& customer : solution.routes[i].customers)
            {
                out << customer.id << " ";
            }
            out << "1 \n";
        }

        out << "\n\n" << "Total solution cost: " << solution.total_cost << "\n";
        out << "\n" << "Total computing time: " << solution.duration_seconds << "s" << "\n";
        out << "\n" << additional_info << "\n";
		out << "\n" << IO_handler_utils::create_table(solution);
        save_progress(solution, add_to_name, ss.str());
        if (!run_results.empty())
        {
            std::string to_remove = "_best_of_";

            std::string new_text = add_to_name;
            size_t pos = new_text.find(to_remove);

            if (pos != std::string::npos) {
                new_text.erase(pos, to_remove.length());
            }

            save_all_costs(run_results, new_text, ss.str());
        }

    }


    void IO_handler::save_all_costs(const std::vector<double>& run_results, std::string add_to_name, std::string time)
    {
        std::filesystem::path folder(result_path);

        std::string filename = add_to_name + "_all_costs_" + time + ".vrp";
        std::filesystem::path file = folder / filename;

        std::filesystem::create_directories(folder);

        std::ofstream out(file);
        if (!out.is_open())
        {
            throw std::runtime_error("Nie mozna otworzyc pliku do zapisu: " + file.string());
        }

        for (int i = 0; i < run_results.size(); i++)
        {
            out << run_results[i] << "\n";
        }
    }


    void IO_handler::save_progress(Result& solution, std::string add_to_name, std::string time)
    {
        std::filesystem::path folder(result_path);

        std::string filename = add_to_name + "_progress_" + time + ".vrp";
        std::filesystem::path file = folder / filename;

        std::filesystem::create_directories(folder);

        std::ofstream out(file);
        if (!out.is_open())
        {
            throw std::runtime_error("Nie mozna otworzyc pliku do zapisu: " + file.string());
        }

        for (int i = 0; i < solution.cost_progress.size(); i++)
        {
            out << solution.cost_progress[i] << "\n";
        } 
    }

    void IO_handler::save_progress(Result& solution)
    {
        if (!save_progress_enabled)
        {
            return;
        }
		namespace fs = std::filesystem;
		fs::create_directories(result_path);
        int file_number = 1;
        for (const auto& entry : fs::directory_iterator(result_path))
        {
            if (entry.path().extension() == ".json") {
                ++file_number;
            }
        }
        fs::path file_path = fs::path(result_path) / ("result_progress_" + std::to_string(file_number) + ".json");

		nlohmann::json j;
		j["total_cost"] = solution.total_cost;

		j["routes"] = nlohmann::json::array();

        for (int i = 0; i < solution.routes.size(); ++i)
        {
            nlohmann::json route;
            route["vehicle_id"] = solution.routes[i].vehicle_id;
            route["remaining_capacity"] = solution.routes[i].remaining_capacity;
            route["route_cost"] = solution.routes[i].route_cost;

            std::vector<int> customer_ids;

   
            for (const auto& customer : solution.routes[i].customers)
            {
				customer_ids.push_back(customer.id);
            }
            route["customers"] = customer_ids;
            route["num_customers"] = customer_ids.size();
            j["routes"].push_back(route);
        }

        std::ofstream out(file_path);
        if (!out.is_open()) {
            throw std::runtime_error("Nie mo¿na otworzyæ pliku do zapisu: " + file_path.string());
        }

        out << j.dump(4); // 4 = wciêcie dla czytelnoœci
        out.close();
    }
}