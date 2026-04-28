#include "settings.h"


Settings::Settings()
{
    number_of_starts_hybrid = 1;
    number_of_starts_skewed = 1;
    number_of_starts_bso = 1;

    num_vehicles = 9;

    bso.T1 = 50;
    bso.T2 = 5;
    bso.alfa_1 = 0.05;
    bso.alfa_2 = 0.05;
    bso.N = 150;
    bso.main_loop_itarations = 4;
    bso.single_route_improvement_margin = 0.01;
    
    hybrid.maxDiv = 130;
    hybrid.maxDiv2 = 110;

    skewed.f_alfa = 500;
    skewed.SVNS_max_no_improve = 300;
    skewed.delta_alfa = 5;

    instance_name = "Golden_1.vrp";
    result_folder_name = "golden1";
    input_path = "InputData/";
    main_result_path = "Results/";
    folder_name = "Test";
}




bool Settings::load_from_file(const std::string& file_name)
{
    std::ifstream file(file_name);
    if (!file.is_open()) {
        std::cerr << "Blad: Nie mozna otworzyc pliku " << file_name << std::endl;
        std::cerr << "Wczytano ustawienia domyslne" << std::endl;
        return false;
    }

    json j;
    file >> j; 

    try {
        
        number_of_starts_hybrid = j.value("number_of_starts_hybrid", number_of_starts_hybrid);
        number_of_starts_skewed = j.value("number_of_starts_skewed", number_of_starts_skewed);
        number_of_starts_bso = j.value("number_of_starts_bso", number_of_starts_bso);

        num_vehicles = j.value("number_of_vehicles", 9);

        // BSO
        if (j.contains("bso_config")) {
         
            auto& b = j["bso_config"];

            bso.main_loop_itarations = b.value("main_loop_iterations", bso.main_loop_itarations);
            bso.T1 = b.value("T1", bso.T1);
            bso.T2 = b.value("T2", bso.T2);
            bso.alfa_1 = b.value("alfa_1", bso.alfa_1);
            bso.alfa_2 = b.value("alfa_2", bso.alfa_2);
            bso.N = b.value("N", bso.N);
            bso.single_route_improvement_margin = b.value("single_route_improvement_margin", bso.single_route_improvement_margin);
        }

       
        // Hybrid
        if (j.contains("hybrid_config")) {
            auto& h = j["hybrid_config"];
            hybrid.maxDiv = h.value("maxDiv", hybrid.maxDiv);
            hybrid.maxDiv2 = h.value("maxDiv2", hybrid.maxDiv2);
        }

        // Skewed
        if (j.contains("skewed_config")) {
            auto& s = j["skewed_config"];
            skewed.f_alfa = s.value("f_alfa", skewed.f_alfa);
            skewed.SVNS_max_no_improve = s.value("SVNS_max_no_improve", skewed.SVNS_max_no_improve);
            skewed.delta_alfa = s.value("delta_alfa", skewed.delta_alfa);
        }

        // Sciezki
        if (j.contains("paths")) {
            auto& p = j["paths"];
            // Dla stringów dzia³a to identycznie
            instance_name = p.value("instance_name", instance_name);
            result_folder_name = p.value("result_folder_name", result_folder_name);
            input_path = p.value("input_path", input_path);
            main_result_path = p.value("main_result_path", main_result_path);
            folder_name = p.value("folder_name", folder_name);
        }

    }
    catch (json::exception& e) {
        std::cerr << "Blad struktury JSON: " << e.what() << std::endl;
        return false;
    }

    return true;
}


