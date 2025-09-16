

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
    std::cerr << "\n------------------------------------------------------- \n";  
}

 
int main()
{
    
    const int num_vehicles = 9;
    IO_handler io_handlers("Golden_1.vrp");
    
   // IO_handler io_handlers("test_data.vrp");
    CVRPInstance input = io_handlers.get_instance();
   
   
    std::vector<int> alfa_values = {6};
    const int runs_per_alfa = 10;
    std::cout << std::boolalpha;


  
	//Result github = io_handlers.load_solution(0,"Golden_1_240.vrp");
    //calculate_cost(github);
	//calculate_remaining_capacity(github);
    //io_handlers.save_solution(github);
    std::cout << "\t\t\t\t\t //==============================================\\\\" << std::endl;
    std::cout << "\t\t\t\t\t ||============Rozpoczecie Skewed_VNS============||" << std::endl;
    std::cout << "\t\t\t\t\t \\\\==============================================//" << std::endl;
    //notepadd ++ \b(\d+)\b(?=.*\b\1\b)
    // 
    // 
    //dla golden1 
    //alfa = 4   wynik _13 = 53542.9    -+-    srednia = 73374.7 - duze wartosci maksymalne 140k
    //alfa = 6   wynik _16 = 56501.9    -+-    srednia = 69370.5 - maksymalnie po 75k 
    //alfa = 8   wynik _17 = 57187.4    -+-    srednia = 68835.4 - maksymalnie 140, 90k na oko tak po 61k 
    //alfa = 10  wynik _14 = 53730.4    -+-    srednia = 66928.1 - maksymalnie jedna/ dwie wartosci po 110k
    //alfa = 20  wynik _15 = 59549.0    -+-    srednia = 68403.9 - maksymalnie jedna/ najwieksza 96k potem ok80k
	//alfa = 50  wynik _18 = 61568.3    -+-    srednia = 66201.4 - maksymalnie jedna/ dwie wartosci po 76k
	//alfa = 100  wynik _19 = 56107.2   -+-    srednia = 71468 
    // 
    // 
    //dla golden2 
	//alfa 120 wynik _7 = 105741    srednia = 117560
	//alfa 80  wynik _6 = 103174    srednia = 123194
	//alfa 50  wynik _5 = 100765    srednia = 117753 
	//alfa 20  wynik _3 = 102798    srednia = 131561 
    //alfa 10  wynik _1 = 103036    srednia = 115867
	//alfa 5   wynik _2 = 102483    srednia = 128794
	//alfa 3   wynik _4 = 101638    srednia = 156498


    //1000 101212 avg=112278
    //2000 102749 avg=116204
  
    for (int f_alfa : alfa_values)
    {
        double total_cost = 0.0;
        double best_cost = std::numeric_limits<double>::max();
        Result best_result;

        std::cout << "\n============== TEST DLA f_alfa = " << f_alfa << " ==============\n";

        for (int i = 0; i < runs_per_alfa; ++i)
        {
            auto start = std::chrono::high_resolution_clock::now();
            loading_done = false;
           // std::thread anim_thread(loading_animation, start);
            Skewed_VNS skewed_vnss(input, num_vehicles);
            skewed_vnss.config.f_alfa = f_alfa;
            skewed_vnss.run();
            Result result = skewed_vnss.get_result();
            loading_done = true;
            //anim_thread.join();
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration = end - start;

            std::cout << "Run " << i + 1 << " | Czas: " << duration.count() / 1000 << " s"
                << " | Koszt: " << result.total_cost
                << " | Przekroczenie pojemnosci: " << has_negtive_capacity(result)
                << "\n";

            if (result.total_cost < best_cost)
            {
                best_cost = result.total_cost;
                best_result = result;
            }

            total_cost += result.total_cost;
        }

        double avg_cost = total_cost / runs_per_alfa;
        std::cout << ">>> f_alfa = " << f_alfa
            << " | Najlepszy koszt: " << best_cost
            << " | Sredni koszt: " << avg_cost
            << "\n";
       
     //   io_handlers.save_solution(best_result);
    }






    




    // Result test = io_handlers.load_solution(2);

   
}

