#pragma once
#include <vector>
#include <math.h>
#include "move.h"	
#include "route.h"	
#include "gainFunctions.h"
#include "random_utils.h"
#include "penalty_functions.h"

//implentacja punktu 3.8  A LNS diversification strategy with a VNS structure - artykul hybrid 198 


//          WAZNE 
//zastosowac staregie usuwana
//usunac wybranych klienow
//przeliczyc wszsytkie dotkniete trasy - czyli nowe koszty, pojemnosci Z1,Z2 itp
//zasosowac metode wstawiania
//po wstawieniu danego klienta przeliczyc trase


//wybieramy lambda klientow (lambda parametr wyliczny na podstawie ilosci wszystkich klientow w rozwiazaniu
//losujemy lambda klientow
//usuwamy lambda klinetow
std::vector<clientInfo> random_removal(std::vector<Route>& solution, int lambda);

//theta parametr 'θ is a user-defined parameter'  = 0.2 z atrykulu
std::vector<clientRatioInfo> worst_removal(std::vector<Route>& solution, double avg_solution_cost, int lambda, double theta = 0.2);
std::vector<clientRatioInfo> worst_distance_removal(std::vector<Route>& solution, int lambda, double theta = 0.2);

//w artukule jest  'conflicting sector removal is π/12'  == 15 stopni
//wyznaczamy sektory o zadanym kacie - sektor to gragment wyznaczany wzgledem calego rozwiazania, a nastepnie liczymy ile roznych pojazdow miesci sie w sektorze, 
//a nastepnie uswamy lambda losowych klientow z sektora ktory ma najwiecej troznych tras
std::vector<clientInfo> confilcting_sector_removal(std::vector<Route>& solution, int lambda, double sector_size_deg  = 15);



//WSTAWIANIE KLIETOW

//zwraca pozycje na jaka nalezy wstawic danego klienta
clientRatioInfo basic_greedy_insertion(std::vector<Route>& solution, Node& new_customer, double avg_cost);

//zwraca clientInfo - gdzie route_index to trasa do ktorej wstawic, client_index to indeks klienta ktorego wstwic
//demand  to indeks klienta ktorego nalzy wstawic z new_customers
clientInfo regret_cost_insertion(std::vector<Route>& solution, std::vector<Node>& new_customers, double avg_cost);


struct PairHash {
    std::size_t operator()(const std::pair<int, int>& p) const {
        return std::hash<int>()(p.first) ^ std::hash<int>()(p.second);
    }
};

//metody na usuwanie zwroca vector z infomacja co mam usunac
//metody na wstawianie zwroca vector z infomracja gdzie mam to wstawic
//a w metoda glowna  3.8 lns diversification - wylosuje metody na wstawianie i usuwanie, nastepnie obliczy uzyskany zysk i jezeli jest lepszy od rozwiazania
//wejsciowego no to dokona zmiany klientow no i zwroci nowe rozwiazanie, jezeli nie ma poprawy to zwroci stare 



