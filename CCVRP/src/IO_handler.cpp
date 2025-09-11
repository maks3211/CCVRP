#include "IO_handler.h"

IO_handler::IO_handler(const std::string& base_name): base_name(base_name) {
	set_base_name(base_name);
}

void IO_handler::set_base_name(const std::string& new_base_name) {
    base_name = new_base_name;
    input_path = "inputData/" + base_name;
	result_path = "Results/" + base_name;
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

bool IO_handler::file_exists(const std::string& file_name)
{
    return std::filesystem::exists(file_name);
}

bool IO_handler::folder_exists(const std::string& folder_name)
{
    return std::filesystem::exists(folder_name);
}

void IO_handler::create_folder_if_not_exists(const std::string& folder) {
    if (!std::filesystem::exists(folder)) {
        std::filesystem::create_directory(folder);
    }
}

void IO_handler::move_file_to_folder(const std::string& file_path, const std::string& folder) {
    std::string filename = std::filesystem::path(file_path).filename().string();
    std::string new_path = folder + "/" + filename;

 
     std::filesystem::rename(file_path, new_path);
}

std::string IO_handler::get_new_filename(const std::string& base_name, const std::string& ext, const std::string& folder) {
    int counter = 1;
    std::string new_name;
    do {
        new_name = folder + "/" + base_name + "_" + std::to_string(counter) + ext;
        counter++;
    } while (std::filesystem::exists(new_name));
    return new_name;
}

CVRPInstance IO_handler::get_instance() 
{
   if(loaded_instance.nodes.empty()) {
       IO_handler::load_instance();
   }
	return loaded_instance;
}

void IO_handler::save_solution(Result &solution)
{
    std::string base_folder = std::filesystem::path(result_path).parent_path().string();
    std::string base_filename = std::filesystem::path(result_path).stem().string();
    std::string extension = std::filesystem::path(result_path).extension().string();

    if (folder_exists(base_folder + "/" + base_filename))
    {
        result_path = get_new_filename(base_filename, extension, base_folder + "/" + base_filename);
    }
    else if (file_exists(result_path)) {
        std::string archive_folder = base_folder + "/" + base_filename;
        create_folder_if_not_exists(archive_folder);
        move_file_to_folder(result_path, archive_folder);
        result_path = get_new_filename(base_filename, extension, archive_folder);
    }




    std::ofstream out(result_path); 
    int ile = 0;
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
	out <<"\n" << create_table(solution);
}
//NOT WORKING YET - TO DO
Result IO_handler::load_solution(int number,std::string name, std::string custom_folder)
{
    std::string filename;
    std::string stem;

    if (name.empty()) {
        filename = base_name;
        stem = std::filesystem::path(base_name).stem().string();
    }
    else {
        filename = name;
        stem = std::filesystem::path(name).stem().string();
    }

    if (number == 0) {
        filename = stem + ".vrp";
    }
    else {
        filename = stem + "_" + std::to_string(number) + ".vrp";
    }

    std::string try1, try2;

    if (!custom_folder.empty()) {
        try1 = "Results/" + custom_folder + "/" + filename;
        //try2 = "";  // pomiń lub zostaw jako zapas
        try2 = "Results/" + custom_folder + "/" + stem + "/" + filename;
    }
    else {
        try1 = "Results/" + filename;
        try2 = "Results/" + stem + "/" + filename;
    }

    if (std::filesystem::exists(try1)) {
        result_path = try1;
    }
    else if (!try2.empty() && std::filesystem::exists(try2)) {
        result_path = try2;
    }
    else {
        throw std::runtime_error("Nie znaleziono pliku rozwiązania: " + try1 + (try2.empty() ? "" : (" ani " + try2)));
    }

    std::ifstream file(result_path);
    if (!file) {
        throw std::runtime_error("Nie można otworzyć pliku: " + result_path);
    }


    std::unordered_map<int, Node> node_map;
    for (const auto& node : get_instance().nodes) {
        node_map[node.id] = node;
    }
    std::string line;
    std::vector<Route> routes;
    double total_solution_cost = 0.0;
    int vehicle_id = 1;
    bool reading_table = false;

    while (std::getline(file, line)) {
        if (line.empty())
            continue;

        // Sekcja tras
        if (line.find("Route") != std::string::npos && line.find(":") != std::string::npos && !reading_table)
        {
            size_t colon_pos = line.find(':');
            std::istringstream iss(line.substr(colon_pos + 1));
            std::vector<int> ids;
            int id;
            while (iss >> id) {
                ids.push_back(id);
            }
            if (ids.empty())
                continue;

            if (ids.back() == loaded_instance.depot_id)
                ids.pop_back();

            Route route(vehicle_id++, loaded_instance.capacity);
            for (int client_id : ids) {
                if (client_id == loaded_instance.depot_id)
                    continue;

                auto it = node_map.find(client_id);
                if (it != node_map.end()) {
                    route.customers.push_back(it->second);
                }
                else {
                    std::cerr << "Brak klienta o ID: " << client_id << " w danych wejściowych.\n";
                }
            }

            // Nie ustawiamy remaining_capacity ani route_cost - będą wczytane z tabeli
            routes.push_back(route);
        }
        else if (line.find("Total solution cost:") == 0)
        {
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string cost_str = line.substr(pos + 1);
                // usuń białe znaki z początku i końca
                cost_str.erase(0, cost_str.find_first_not_of(" \t"));
                cost_str.erase(cost_str.find_last_not_of(" \t") + 1);

                total_solution_cost = std::stod(cost_str);
            }
        }

        // Po sekcji tras szukamy nagłówka tabeli
        else if (line.find("Route | Remaining capacity") != std::string::npos) {
            reading_table = true;

            // Pomijamy linię separatora
            std::getline(file, line);
            continue;
        }
        // Czytamy wiersze tabeli
        else if (reading_table) {
            if (line.find("---") != std::string::npos) // linia separatora - pomijamy
                continue;
            if (line.empty()) // koniec tabeli
                break;

            // Parsujemy wiersz tabeli
            std::istringstream iss(line);
            int route_num;
            int rem_capacity;
            double route_cost;
            int num_clients;

            char sep; // do pomijania separatorów '|'
            iss >> route_num >> sep;

            // Usuwamy spacje i wczytujemy pola oddzielone '|'
            // Ponieważ format jest stały, można czytać pola z '|' jako separator

            std::string rem_cap_str, route_cost_str, num_clients_str;

            std::getline(iss, rem_cap_str, '|');
            std::getline(iss, route_cost_str, '|');
            std::getline(iss, num_clients_str, '|');

            // Usuwamy białe znaki
            auto trim = [](std::string& s) {
                s.erase(0, s.find_first_not_of(" \t\n\r\f\v"));
                s.erase(s.find_last_not_of(" \t\n\r\f\v") + 1);
                };

            trim(rem_cap_str);
            trim(route_cost_str);
            trim(num_clients_str);

            rem_capacity = std::stoi(rem_cap_str);
            route_cost = std::stod(route_cost_str);
            num_clients = std::stoi(num_clients_str);

            // Aktualizujemy odpowiednią trasę
            if (route_num > 0 && route_num <= (int)routes.size()) {
                routes[route_num - 1].remaining_capacity = rem_capacity;
                routes[route_num - 1].route_cost = route_cost;
              
            }
        }
    }
    Result res;
	res.routes = routes;
	res.total_cost = total_solution_cost;
	return res;

}


std::string IO_handler::create_table(Result& solution)
{
    std::vector<std::string> table_headers = {"Route", "Remaining capacity", "Route cost", "Number of clients"};

    int column_number = solution.routes.size() + 1;
    std::vector<int> column_widths;

    int column_width = 0;
    for (auto e : table_headers)
    {
        column_widths.push_back(e.size());
    }

    ////////////////////////////
    std::ostringstream oss;
	int total_width = 0;
    for (auto a : column_widths)
    {
        total_width += a; // +3 for padding and separator
    }


    // Naglowki tabeli
    for (int i = 0; i < table_headers.size(); i++)
    {
		oss << std::setw(column_widths[i]) << std::left << table_headers[i];
        if (i < table_headers.size() - 1) oss << " | ";
    }
    oss << "\n";
	draw_horizontal_lines(oss, total_width);
	oss << "\n";


    for (int s = 0; s < solution.routes.size(); s++)
    {
		oss << std::setw(column_widths[0]) << std::left << solution.routes[s].vehicle_id << " | ";
		oss << std::setw(column_widths[1]) << std::left << solution.routes[s].remaining_capacity << " | ";
		oss << std::setw(column_widths[2]) << std::left << solution.routes[s].route_cost << " | ";
		oss << std::setw(column_widths[3]) << std::left << solution.routes[s].customers.size() << " | ";
        oss << "\n";
        draw_horizontal_lines(oss, total_width);
        oss << "\n";
    }

    std::string result = oss.str();
    std::cout << result;
    return result;


}

void IO_handler::draw_horizontal_lines(std::ostringstream& oss, int width)
{
    for (int i = 0; i < width + 10; i++)
    {
		oss << "-"; 
    }
}