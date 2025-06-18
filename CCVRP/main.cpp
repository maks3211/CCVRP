

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

    std::cerr << "\rOperacja zakonczona.                    \n";  
}

 
int main()
{
    
    const int num_vehicles = 10;
	IO_handler io_handlers("Golden_2.vrp");
    CVRPInstance input = io_handlers.get_instance();
	int f_alfa = 45;





	//Result github = io_handlers.load_solution(0,"Golden_1_240.vrp");
    //calculate_cost(github);
	//calculate_remaining_capacity(github);
    //io_handlers.save_solution(github);
    std::cout << "=============Rozpoczecie Skewed_VNS=============" << std::endl;
    //notepadd ++ \b(\d+)\b(?=.*\b\1\b)
    //dla golden1 
    //alfa = 4   wynik _13 = 53542.9    -+-    srednia = 73374.7 - duze wartosci maksymalne 140k
    //alfa = 6   wynik _16 = 56501.9    -+-    srednia = 69370.5 - maksymalnie po 75k 
    //alfa = 8   wynik _17 = 57187.4    -+-    srednia = 68835.4 - maksymalnie 140, 90k na oko tak po 61k 
    //alfa = 10  wynik _14 = 53730.4    -+-    srednia = 66928.1 - maksymalnie jedna/ dwie wartosci po 110k
    //alfa = 20  wynik _15 = 59549.0    -+-    srednia = 68403.9 - maksymalnie jedna/ najwieksza 96k potem ok80k

    //dla golden2 
	//alfa 120 wynik _7 = 105741    srednia = 117560
	//alfa 80  wynik _6 = 103174    srednia = 123194
	//alfa 50  wynik _5 = 100765    srednia = 117753 
	//alfa 20  wynik _3 = 102798    srednia = 131561 
    //alfa 10  wynik _1 = 103036    srednia = 115867
	//alfa 5   wynik _2 = 102483    srednia = 128794
	//alfa 3   wynik _4 = 101638    srednia = 156498
    double best_cost = 110684;
	Result best_result;
    double total_cost = 0;
	best_result.total_cost = best_cost;
    std::cout << std::boolalpha;
    for (int i = 0; i < 20; i++)
    {
        auto start = std::chrono::high_resolution_clock::now();
		loading_done = false;
        std::thread anim_thread(loading_animation, start);
        Skewed_VNS skewed_vnss(input, num_vehicles);
        skewed_vnss.config.f_alfa = f_alfa;
        skewed_vnss.run();
        Result result = skewed_vnss.get_result();

        loading_done = true;
        anim_thread.join();

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        std::cout  << i << "=================  CZAS WYKONANIA: " << duration.count() / 1000 << "s   (" << duration.count() / 1000 / 60 << " min) ==================\n\n";
        std::cout << "Wyznaczony koszt rozwiazania:  " << result.total_cost << std::endl;
        if (result.total_cost < best_result.total_cost)
        {
            best_result = result;
        }
		total_cost += result.total_cost;
        std::cout << "Przekroczona pojemnosc: " << has_negtive_capacity(result);
        std::cout << "=================Sredni koszt rozwiazania:  " << total_cost / (i + 1) << "=================\n";
    }
    
    std::cout << "=+=+=+=+=+=+=+=+=+=+=+KONIEC+=+=+=+=+=+=+=+=+=+=+=\n\n";
    io_handlers.save_solution(best_result);

    
    std::cout << "Najlepszy wyznaczony koszt rozwiazania:  " << best_result.total_cost;



    // Result test = io_handlers.load_solution(2);

   
}

