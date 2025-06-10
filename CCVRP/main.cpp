

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

std::atomic<bool> loading_done(false);

void loading_animation(std::chrono::high_resolution_clock::time_point start_time) {  
    const char animation_chars[] = { '|', '/', '-', '\\' };  
    int animation_size = sizeof(animation_chars) / sizeof(animation_chars[0]); // Calculate the size of the array  
    int i = 0;  

    while (!loading_done) {  
        auto now = std::chrono::high_resolution_clock::now();  
        auto elapsed = std::chrono::duration<double>(now - start_time).count(); // sekundy  

        std::cerr << "\r[" << animation_chars[i++ % animation_size] << "] Czas dzialania: "  
            << std::fixed << std::setprecision(2) << elapsed << " s" << std::flush;  

        std::this_thread::sleep_for(std::chrono::milliseconds(250));  
    }  

    std::cerr << "\rOperacja zakończona.                    \n";  
}

 

// !!!!!!!!!!!!!NIGDZIE PRZY PRZESTAWIANIU KLIENTOW NIE AKTUALIZUJE POJEMNOSCI POJAZDOW !!!!!!!!!!!!!
int main()
{
    
    const int num_vehicles = 9;
	IO_handler io_handlers("Golden_1.vrp");
    CVRPInstance input = io_handlers.get_instance();

    std::cout << "=============Rozpoczecie Skewed_VNS=============" << std::endl;

    
    auto start = std::chrono::high_resolution_clock::now();
    std::thread anim_thread(loading_animation, start);
    Skewed_VNS skewed_vnss(input, num_vehicles);
    skewed_vnss.run();
	Result result = skewed_vnss.get_result();

	loading_done = true;
    anim_thread.join();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "==================  CZAS WYKONANIA: " << duration.count() / 1000 << "s   ("<< duration.count() / 1000 / 60 <<" min) ==================\n";


	io_handlers.save_solution(result);
    std::cout << "Wyznaczony koszt rozwiazania:  " << result.total_cost;



    // Result test = io_handlers.load_solution(2);

   
}

