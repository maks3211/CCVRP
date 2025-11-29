

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

    
    const int num_vehicles = 9;
    IO_handlerV1::IO_handler io_handlers("Golden_1.vrp");


	IO_handlerV2::IO_handler io_handlers_v2;
	io_handlers_v2.set_result_path("Results/test2");
	io_handlers_v2.set_input_path("InputData/Golden_1.vrp");
    
    CVRPInstance input = io_handlers_v2.get_instance();
    //CVRPInstance input = io_handlers.get_instance();
   
   
    std::vector<int> alfa_values = {5};
    const int runs_per_alfa = 1;
    std::cout << std::boolalpha;

    Result github = io_handlers.load_solution(0, "Golden_1.vrp");

    //for (int i = 0; i < github.routes.size(); i++)
    //{
    //  //  github.routes[i].customers.erase(github.routes[i].customers.begin());
    //    github.routes[i].customers.push_back(github.routes[i].customers[0]);
    //}
    calculate_cost(github);
   calculate_remaining_capacity(github);
     
	//Result github = io_handlers.load_solution(0,"Golden_1_240.vrp");
    //calculate_cost(github);
	//calculate_remaining_capacity(github);
    //io_handlers.save_solution(github);

    //notepadd ++ \b(\d+)\b(?=.*\b\1\b)
 
    std::cout << "\t\t\t\t\t //==============================================\\\\" << std::endl;
    std::cout << "\t\t\t\t\t ||============Rozpoczecie Skewed_VNS============||" << std::endl;
    std::cout << "\t\t\t\t\t \\\\==============================================//" << std::endl;
    for (int f_alfa : alfa_values)
    {
		double total_duration_seconds = 0.0;
        double total_cost = 0.0;
        double best_cost = std::numeric_limits<double>::max();
        Result best_result;

        std::cout << "\n============== TEST DLA f_alfa = " << f_alfa << " ==============\n";

        for (int i = 0; i < runs_per_alfa; ++i)
        {
            auto start = std::chrono::high_resolution_clock::now();
            loading_done = false;

#ifndef _DEBUG
            std::thread anim_thread(loading_animation, start);
#endif // !_DEBUG

          

            Skewed_VNS skewed_vnss(input ,num_vehicles, io_handlers_v2);
            skewed_vnss.config.f_alfa = f_alfa;
			skewed_vnss.config.SVNS_max_no_improve = 1000;
            skewed_vnss.run();
            Result result = skewed_vnss.get_result();

            loading_done = true;
#ifndef _DEBUG
            anim_thread.join();
#endif
         

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration = end - start;
			result.duration_seconds = duration.count() / 1000.0; // sekundy
            bool exceeded = has_negtive_capacity(result);
            if (exceeded) {
                std::cout << "\033[31m";
            }

            std::cout << "Run " << i + 1
                << " | Czas: " << duration.count() / 1000 << " s"
                << " | Koszt: " << result.total_cost
                << " | Przekroczenie pojemnosci: "
                << (exceeded ? "TAK" : "NIE")
                << "\033[0m\n";  // reset na końcu

            std::cout << "czy sie powtarza: " << any_global_duplicates(result.routes) <<"ile wolnego: " << get_total_remaining_capacity(result) << " ile po " << calculate_remaining_capacity(result);
		
          
            if (result.total_cost < best_cost)
            {
                best_cost = result.total_cost;
                best_result = result;
            }

            total_cost += result.total_cost;
			total_duration_seconds += result.duration_seconds;

              
        }

        double avg_cost = total_cost / runs_per_alfa;
		double avg_time = total_duration_seconds / runs_per_alfa;
        std::cout << "\033[32m"  // włącz zielony
            << ">>> f_alfa = " << f_alfa
            << " | Najlepszy koszt: " << best_cost
            << " | Sredni koszt: " << avg_cost
			<< " | Sredni czas: " << avg_time << "s"
            << "\033[0m"   // reset koloru (wraca do domyślnego)
            << "\n";
       
        //io_handlers.save_solution(best_result);
		io_handlers_v2.save_solution(best_result);
		//io_handlers_v2.save_progress(best_result);
    }






    




    // Result test = io_handlers.load_solution(2);

   
}

