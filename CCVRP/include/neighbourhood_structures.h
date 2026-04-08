#pragma once
#include <vector>
#include <algorithm>
#include<iostream>
#include "route.h"
#include "cvrp_types.h"
#include "random_utils.h"
#include "utils.h"

//Neighbourhood structures z artykulu hybrydu pkt 3.6
//donor - trasa z ktorej bierzemy klientow (losowo wybierani)
// receiver - trasa do ktorej wstawiamy
// sprawdzamy pozycje do poki nie napotamy pierwszego dobrego wstawienia (czyli takiego ktorego nie narusza pojemnosci
//


//dwoch losowych sasiadow usuwanych i wstawianych do dwoch osobnych, losowych tras (w praktyce zawsze wstawiamy na pierwsza pozycje, bo sprawdzamy tylko pojemnos)
std::vector<Route> two_insertion(std::vector<Route>& current_solution);


// dwoch losowych klientow z jednej trasy A jest usuwanych 
//pierwszego z nich wstawiamy do innej losowej trasy (trafia zawsze na pierwsza pozycje, ablo wcale)
//drugiego klienta z trasy A zaminiamy z losowym klientem z trzeciej trasy 
std::vector<Route> two_one_interchange(std::vector<Route>& current_solution);


//wybieramy s sasiadow  (s = od 0.5nk do 0.7nk) i losowo zmieniamy ich kolejnosc
//czyli po prostu tasujemy s sasiadow w tej samej trasie, integujac tylko w ten fragment
//jako jedyna metoda dziala tylko w obrebie jednej trasy
//nk liczba klientow w trasie k
std::vector<Route> segment_reshuffle(std::vector<Route>& current_solution, int nk = -1);


//wymianiamy fragment m1 z trasy j z framgentem m2 z trasy k 
//wartosci losowo wybierane z  m1 = 0.2nj do 0.4nj , m2 = 0.2nk do 0.4nk
//nj, nk jako parametr raczej nie ma sensu bo i tak sa to losowe trasy
std::vector<Route> cross_exchange(std::vector<Route>& current_solution, int nj = -1, int nk = -1);


//bierzemy pioerwsze polowy dwoch losowych tras i zamieniamy je miejscami
//te polowy moga miec rozna dlugosc, no ale zawsze jest to polowa trasy
std::vector<Route> head_swap(std::vector<Route>& current_solution);


//wywoluje sasiedztwo na podstawie jego numeru, nuermacja zgodnie z kolejnoscia w punkcie 3.6  artukul 198
std::vector<Route> S_p_neighbourhood(int op_id, std::vector<Route>& solution);
