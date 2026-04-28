

#include <iostream>
#include <fstream> 
#include <algorithm>
#include <random>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <unordered_set>
#include "cvrp_parser.h"
#include "include/route.h"
#include "ResultsParser.h"
#include <chrono>
#include "include/cvrp_types.h"
#include "include/skewed_vns.h"
#include "include/IO_handler.h"
#include "include/IO_handlerV2.h"
#include "include/hybridAvnsLns.h"
#include "include/bso.h"
#include "include/ui.h"
#include "include/settings.h"

//GLOBAL CONFIG VARIABLES



std::atomic<bool> loading_done(false);

void loading_animation(std::chrono::high_resolution_clock::time_point start_time) {  
    const char animation_chars[] = { '|', '/', '-', '\\' };  
    int animation_size = sizeof(animation_chars) / sizeof(animation_chars[0]); 
    int i = 0;  

    while (!loading_done) {  
        auto now = std::chrono::high_resolution_clock::now();  
        auto elapsed = std::chrono::duration<double>(now - start_time).count(); // sekundy  

        std::cerr << "\r[" << animation_chars[i++ % animation_size] << "] Czas dzialania: "  
            << std::fixed << std::setprecision(2) << elapsed << " s" << std::flush;  

        std::this_thread::sleep_for(std::chrono::milliseconds(250));  
    }  
    std::cerr << "\n------------------------------------------------------- \n";  
}

 
int main()
{
    bool run_Skewed_VNS = true;
    bool run_hybrid = false;
    bool run_bso = false;


    Settings s;
    bool config_loaded = s.load_from_file("settings.json");

    

    //=========== LODING ANIMATION ===========
    bool loading_animation_enabled = true;
    std::vector<double> run_results;
    
   
   
 
    


	IO_handlerV2::IO_handler io_handlers_v2;
    io_handlers_v2.save_progress_enabled = false;

    //=========== USTAWIENIA WEJSCIA ===========
    std::string full_input_path = s.input_path + s.instance_name;

   
    //CVRPInstance input = io_handlers.get_instance();
   


    Ui ii(s.bso, s.hybrid, s.skewed, 
        s.number_of_starts_bso, s.number_of_starts_hybrid, s.number_of_starts_skewed,
        s.input_path, s.instance_name, s.num_vehicles,
        s.main_result_path, s.folder_name, config_loaded);
    ii.main_menu();
    full_input_path = s.input_path + s.instance_name;

    std::cout << "Input: " << full_input_path << "\n";
    //=========== USTAWIENIA ZAPISU ===========

    bool input_path_bool = io_handlers_v2.set_input_path(full_input_path);
    std::cout << "Input path set: " << std::boolalpha << input_path_bool << std::endl;
    if (!input_path_bool)
    {
        std::cout << "\nBledna sciezka wejsciowa";
            return -1;
    }

    CVRPInstance input = io_handlers_v2.get_instance();


    run_bso = (s.number_of_starts_bso != 0) ? true : false;
    run_hybrid = (s.number_of_starts_hybrid != 0) ? true : false; 
    run_Skewed_VNS = (s.number_of_starts_skewed != 0) ? true : false;



    size_t pos = s.instance_name.find_last_of('.');
    if (pos != std::string::npos) {
        s.result_folder_name = s.instance_name.substr(0, pos);
    }
    else {
        s.result_folder_name = s.instance_name; // brak rozszerzenia
    }

 
	//END CONFIGURATION
    std::cout << std::boolalpha;
    //                                                  =========BSO=========
    if (run_bso)
    {
        std::cout << "\n                =========== ROZPOCZECIE BSO ===========\n";
        std::string full_result_path = s.main_result_path + "bso/" + s.folder_name + "/" + s.result_folder_name;
        std::cout << "\nSciezka zapisu: " << full_result_path;
        bool res_path = io_handlers_v2.set_result_path(full_result_path);
        std::cout << "\nResult path set: " << std::boolalpha << res_path << std::endl;
        std::string file_info = "bso_" + s.result_folder_name;

        
        double avg_bso_cost = 0.0;
        double avg_bso_time = 0.0;
        double best_bso_cost = 99999999.0;
        double worst_bso_cost = 0.0;
        Result best;

        for (int rounds = 0; rounds < s.number_of_starts_bso; rounds++)
        {
            
            BrainStormOptimalization bso(input, s.num_vehicles, io_handlers_v2, s.bso);
            loading_done = false;
            auto start = std::chrono::high_resolution_clock::now();
#ifndef _DEBUG
            std::thread anim_thread;
            if (loading_animation_enabled) {
                anim_thread = std::thread(loading_animation, start);
            }
#endif  // !_DEBUG
            bso.run();
            loading_done = true;
            auto end = std::chrono::high_resolution_clock::now();
#ifndef _DEBUG
            if (loading_animation_enabled && anim_thread.joinable()) {
                anim_thread.join();
            }
#endif
            Result result = bso.get_result();        
            std::chrono::duration<double, std::milli> duration = end - start;
            result.duration_seconds = duration.count() / 1000.0; // sekundy


            bool exceeded = has_negtive_capacity(result);
           
            if (exceeded) {
                std::cout << "\033[31m";
            }

            std::cout
                << "#" << rounds << "\n"
                << " | Czas: " << duration.count() / 1000 << " s"
                << " | Koszt: " << result.total_cost
                << " | Przekroczenie pojemnosci: "
                << (exceeded ? "TAK" : "NIE")
                << "\033[0m\n";  // reset na końcu

            std::cout << "Czy sa duplikaty: " << any_global_duplicates(result.routes) << " ile wolnego : " << get_total_remaining_capacity(result) << " ile po ";// << calculate_remaining_capacity(hybridResult);

            std::cout << "\n        ZAKONCZONO\n";

            std::string additional_info =
                "T1 = " + std::to_string(s.bso.T1) +
                ", Main loop iterations: " + std::to_string(s.bso.main_loop_itarations);
            run_results.push_back(result.total_cost);
            io_handlers_v2.save_solution(result, file_info, additional_info);
            avg_bso_cost += result.total_cost;
            avg_bso_time += result.duration_seconds;
            if (best_bso_cost > result.total_cost)
            {
                best = result;
                best_bso_cost = result.total_cost;
            }

            if(result.total_cost > worst_bso_cost) // zapisane najgorszego wyniku
            {
               worst_bso_cost = result.total_cost;
            }
          
        }

        avg_bso_cost /= s.number_of_starts_bso;
        avg_bso_time /= s.number_of_starts_bso;
          io_handlers_v2.set_result_path(s.main_result_path + "bso/best");
          file_info = "bso_" + s.result_folder_name + "_best_of_" + std::to_string(s.number_of_starts_bso);

        std::cout << "\n======= Zakonczono bso =======" << "\n\nNajlepsze rozwiazanie: " << best_bso_cost;
        std::cout << "\nSredni koszt z " << s.number_of_starts_bso << " rozwiazan: " << avg_bso_cost << "\n";
        std::string additional_info =
            "T1 = " + std::to_string(s.bso.T1) +
            ", T2 = " + std::to_string(s.bso.T2) +
            ", Main loop iterations: " + std::to_string(s.bso.main_loop_itarations) + 
            "\nAverage cost of solution from: " + std::to_string(s.number_of_starts_bso) +
            " attempts: " + std::to_string(avg_bso_cost) + "\nAverage computing time from: " + std::to_string(s.number_of_starts_bso) + " attempts: " + std::to_string(avg_bso_time) + "s" + "\n Worst solution cost: " + std::to_string(worst_bso_cost);
        io_handlers_v2.save_solution(best, file_info, additional_info, run_results);

        std::cout << "\n                =========== KONIEC BSO ===========\n";
    }

   

    //                                                  =========HYBRID=========
    if (run_hybrid)
    {
        run_results.clear();
        std::cout << "\n                =========== ROZPOCZECIE HYBRID ===========\n";
        std::string full_result_path = s.main_result_path + "hybrid/" + s.folder_name + "/" + s.result_folder_name;
        std::cout << "\nSciezka zapisu: " << full_result_path;
        bool res_path = io_handlers_v2.set_result_path(full_result_path);
      
        std::cout << "Result path set: " << std::boolalpha << res_path << std::endl;
        std::string file_info = "hybrid_" + s.result_folder_name;
        
        double avg_hybrid_cost = 0.0;
        double avg_hybrid_time = 0.0;
        double best_hybrid_cost = 99999999.0;
        double worst_hybrid_cost = 0.0;
        Result best;
        //Uruchamia number_of_starts prob dla hybrid, zapisuje kazda z prob w folderze hybrid/final oraz zapisuje best of number_of_starts w hybrid
        for (int rounds = 0; rounds < s.number_of_starts_hybrid; rounds++)
        {
            HybridAvnsLns hybrid(input, s.num_vehicles, io_handlers_v2, s.hybrid);

            loading_done = false;
            auto start = std::chrono::high_resolution_clock::now();
#ifndef _DEBUG
            std::thread anim_thread;
            if (loading_animation_enabled) {
                anim_thread = std::thread(loading_animation, start);
            }
#endif  // !_DEBUG
          hybrid.run();
          auto end = std::chrono::high_resolution_clock::now();
            loading_done = true;
#ifndef _DEBUG
            if (loading_animation_enabled && anim_thread.joinable()) {
                anim_thread.join();
            }
#endif
            Result hybridResult = hybrid.get_result();
            
            std::chrono::duration<double, std::milli> duration = end - start;
            hybridResult.duration_seconds = duration.count() / 1000.0; // sekundy


            bool exceeded = has_negtive_capacity(hybridResult);
            std::cout << "\nPozstala pojemnosc [1]: " << hybridResult.routes[1].remaining_capacity << std::endl;
            if (exceeded) {
                std::cout << "\033[31m";
            }

            std::cout
                << "#" << rounds << "\n"
                << " | Czas: " << duration.count() / 1000 << " s"
                << " | Koszt: " << hybridResult.total_cost
                << " | Przekroczenie pojemnosci: "
                << (exceeded ? "TAK" : "NIE")
                << "\033[0m\n";  // reset na końcu

            std::cout << "Czy sa duplikaty: " << any_global_duplicates(hybridResult.routes) << " ile wolnego : " << get_total_remaining_capacity(hybridResult) << " ile po ";// << calculate_remaining_capacity(hybridResult);

            std::cout << "\nZAKONCZONO run\n";

            std::string additional_info =
                "MaxDiv = " + std::to_string(s.hybrid.maxDiv) +
                ", MaxDiv2 = " + std::to_string(s.hybrid.maxDiv2);
            run_results.push_back(hybridResult.total_cost);
            io_handlers_v2.save_solution(hybridResult, file_info, additional_info);
            avg_hybrid_cost += hybridResult.total_cost;
            avg_hybrid_time += hybridResult.duration_seconds;
            if (best_hybrid_cost > hybridResult.total_cost)
            {
                best = hybridResult;
                best_hybrid_cost = hybridResult.total_cost;
            }
            if (hybridResult.total_cost > worst_hybrid_cost) // zapisane najgorszego wyniku
            {
                worst_hybrid_cost = hybridResult.total_cost;
            }
         
        }

        avg_hybrid_cost /= s.number_of_starts_hybrid;
        avg_hybrid_time /= s.number_of_starts_hybrid;
        io_handlers_v2.set_result_path(s.main_result_path + "hybrid/best");
        file_info = "hybrid_" + s.result_folder_name + "_best_of_" + std::to_string(s.number_of_starts_hybrid);

        std::cout << "\n======= Zakonczono hybrid =======" << "\n\nNajlepsze rozwiazanie: " << best_hybrid_cost;
        std::cout << "\nSredni koszt z " << s.number_of_starts_hybrid << " rozwiazan: " << avg_hybrid_cost << "\n";
        std::string additional_info =
            "MaxDiv = " + std::to_string(s.hybrid.maxDiv) +
            " ,MaxDiv2 = " + std::to_string(s.hybrid.maxDiv2) +
            "\nAverage cost of solution from: " + std::to_string(s.number_of_starts_hybrid) +
            " attempts: " + std::to_string(avg_hybrid_cost) + "\nAverage computing time from: " + std::to_string(s.number_of_starts_hybrid) + " attempts: " + std::to_string(avg_hybrid_time) + "s" + "\n Worst solution cost: " + std::to_string(worst_hybrid_cost);
        io_handlers_v2.save_solution(best, file_info, additional_info, run_results);
        std::cout << "\n                =========== KONIEC HYBRID ===========\n";
    }
   
   
    //                                                  =========SKEWED=========
    if (run_Skewed_VNS)
    {
        run_results.clear();
        std::cout << "\n                =========== ROZPOCZECIE SKEWED ===========\n";
        std::string full_result_path = s.main_result_path + "skewed/" + s.folder_name + "/" + s.result_folder_name;
        std::cout << "\nSciezka zapisu: " << full_result_path;
        bool res_path = io_handlers_v2.set_result_path(full_result_path);
        std::cout << "Result path set: " << std::boolalpha << res_path << std::endl;
        std::string file_info = "skewed_" + s.result_folder_name;


        double avg_skewed_cost = 0.0;
        double avg_skewed_time = 0.0;
        double best_skewed_cost = 99999999.0;
        double worst_skewed_cost = 0.0;
        Result best;

        for (int rounds = 0; rounds < s.number_of_starts_skewed; rounds++)
        {
            Skewed_VNS skewed_vnss(input, s.num_vehicles, io_handlers_v2, s.skewed);

            loading_done = false; 
            auto start = std::chrono::high_resolution_clock::now();

#ifndef _DEBUG
            std::thread anim_thread;
            if (loading_animation_enabled) {
                anim_thread = std::thread(loading_animation, start);
            }
#endif  // !_DEBUG

            skewed_vnss.run();
            loading_done = true;
            auto end = std::chrono::high_resolution_clock::now();

#ifndef _DEBUG
            if (loading_animation_enabled && anim_thread.joinable()) {
                anim_thread.join();
            }
#endif
         
            Result result = skewed_vnss.get_result();
            std::chrono::duration<double, std::milli> duration = end - start;
            result.duration_seconds = duration.count() / 1000.0; // sekundy

            bool exceeded = has_negtive_capacity(result);
            if (exceeded) {
                std::cout << "\033[31m";
            }

            std::cout
                << "#" << rounds << "\n"
                << " | Czas: " << duration.count() / 1000 << " s"
                << " | Koszt: " << result.total_cost
                << " | Przekroczenie pojemnosci: "
                << (exceeded ? "TAK" : "NIE")
                << "\033[0m\n";  // reset na koncu
            std::cout << "Czy sa duplikaty: " << any_global_duplicates(result.routes) << " ile wolnego : " << get_total_remaining_capacity(result) << " ile po ";// << calculate_remaining_capacity(hybridResult);


            std::string additional_info =
                "F_alfa =  " + std::to_string(s.skewed.f_alfa);
            run_results.push_back(result.total_cost);
            io_handlers_v2.save_solution(result, file_info, additional_info);
            avg_skewed_cost += result.total_cost;
            avg_skewed_time += result.duration_seconds;
            if (best_skewed_cost > result.total_cost)
            {
                best = result;
                best_skewed_cost = result.total_cost;
            }
            if (result.total_cost > worst_skewed_cost) // zapisane najgorszego wyniku
            {
                worst_skewed_cost = result.total_cost;
            }
         
        }

        avg_skewed_cost /= s.number_of_starts_skewed;
        avg_skewed_time /= s.number_of_starts_skewed;
        io_handlers_v2.set_result_path(s.main_result_path + "skewed/best");
        file_info = "skewed_" + s.result_folder_name + "_best_of_" + std::to_string(s.number_of_starts_skewed);

        std::cout << "\n======= Zakonczono skewed =======" << "\n\nNajlepsze rozwiazanie: " << best_skewed_cost;
        std::cout << "\nSredni koszt z " << s.number_of_starts_skewed << " rozwiazan: " << avg_skewed_cost << "\n";
        std::string additional_info =
            "F_alfa =  " + std::to_string(s.skewed.f_alfa) +
            "\nSVNS_max_no_improve =  " + std::to_string(s.skewed.SVNS_max_no_improve) +
            "\nAverage cost of solution from: " + std::to_string(s.number_of_starts_skewed) +
            " attempts: " + std::to_string(avg_skewed_cost) + "\nAverage computing time from: " + std::to_string(s.number_of_starts_skewed) + " attempts: " + std::to_string(avg_skewed_time) + "s" + "\n Worst solution cost: " + std::to_string(worst_skewed_cost);;
        io_handlers_v2.save_solution(best, file_info, additional_info, run_results);

        std::cout << "\n                =========== KONIEC SKEWED ===========\n";
    }   
}

