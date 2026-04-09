#pragma once 
#include <vector>
#include "move.h"	
#include "route.h"	
#include "gainFunctions.h"
//brak pliku .cpp ze wzgledu na obecnosc template - wtedy sie stoduje chyba pliki tpp, no ale zrobione jest wszystko w jednym .h
//INTRA - jedna trasa
//INTER - dwie rozne trasy


struct BestMoves {
    std::vector<Move> topK;   // trzymamy tylko K najlepszych
    double worst_in_topK = -std::numeric_limits<double>::infinity(); //informacja ile wynosi najgorzszy z K wynikow
	int L_h = 0; // numer operatora
};


// 
//
//=============================== IMPLENTACJA LOCAL SERACH ENGINE DLA Hybryy - 198 ===============================
// 
// 
 


//wywolanie metody Lh 

/// <summary>
/// 
/// </summary>
/// <param name="solution"></param>
/// <param name="h">Numer metody</param>
/// <param name="k">Ile najlepszych ruchow ma zwrocic</param>
/// <returns></returns>


//oznaczone jako Lh

//uzycie - get_top_k_1insertion_moves(solution, k, g) - zwraca k najlepszych ruchow 1-insertions dla danej solucji i funkcji zysku g
//gdzie g to funkcja double g(sol 1, sol 2)...

//both intra and inter
//Trzy to parmetr z artykulu (strona 18 - we define K = 3 for kth best improvement)
//przeniosimy klienta na kazda inna mozliwa pozycje w obrebie wszystkich tras (oraz w tej samej) -
//czyli w praktyce sprawdzam wszystkie mozliwe pozycje w calym rozwiazaniu)
//czyli to bedzie korzystac z funckji - w momencie wywolywania tego ruchu to musze przekazac calculate_gain_1_insertion_gain_hybrid 



bool update_best_k_moves(BestMoves& best, double gain, int k, Move& move);

//wykorzystane w cross_tail
//calculate_crosstail_gain zwraca vector trzech ruchow - czyli dla obu odwroconych, i jednego z dwoch odwroconych

bool update_best_k_moves_from_vector(BestMoves& best, int k, std::vector<Move>& moves);

//												PIERWSZY OPERATOR 
//usuwam jednego klienta i wstwiam go na kazda inna mozliwa pozycje we wszystkioch mozliwych trasach

BestMoves get_top_k_1_insertion_moves(std::vector<Route>& solution, int k = 3);

//both intra - one route  and inter two routes 
//zaminiamy miejscami dwoch klientow 
BestMoves get_top_k_1_1_exchange_moves(std::vector<Route>& solution, int k = 3);


//Usuniecie dwoch kolejnych klientow (sasiadow) z jednej trasy i umieszczenie ich w innym mijescu
// w tej samej lub innej trasie oraz w tej samej lub odwroconej kolejnosci
//both intra and inter

BestMoves get_top_k_2_insertion_moves(std::vector<Route>& solution, int k);


//Intra only - dzilanie w obrebie jednej trasy
//wybieram dwa rozne punkty na trasie i odwracam caly fragment miedzy nimi
BestMoves get_top_k_2_opt_moves(std::vector<Route>& solution, int k);

//Inter only - dzialanie miedzy dwiema roznymi trasami, nie mozna przestawiac klientow w obrebie jednej trasy
//zamieniem ogony dwoch tras - dlugosc ogonow oraz miejsce ciecia moze byc pomiedzy trasami
BestMoves get_top_k_2_opt_prim_moves(std::vector<Route>& solution, int k);

//Inter only - dzialanie miedzy dwiema roznymi trasami, nie mozna przestawiac klientow w obrebie jednej trasy
//analogicznie do 2_opt* ale tutaj dodatkowo te przestawiane framgenty moga byc odwracane - czyli zamieniam ogony dwoch tras ale dodatkowo moge odwracac te ogony
//wiec w praktyce najpierw licze 2_opt* a potem dla tych samych punktow ciecia licze 2_opt* ale z odwracaniem fragmentow (rev/org, rev/rev, org/rev)
BestMoves get_top_k_cross_tail_moves(std::vector<Route>& solution, int k);

BestMoves L_h_local_serach(std::vector<Route>& solution, int h, int k);

std::vector<Route> perform_local_move(std::vector<Route>& solution, Move move, int structure, double avg_cost);


std::vector<Route> perform_1_insertion_move(std::vector<Route>& solution, Move move, double avg_cost);
std::vector<Route> perform_1_1_exchange_move(std::vector<Route>& solution, Move move, double avg_cost);
std::vector<Route> perform_2_insertion_move(std::vector<Route>& solution, Move move, double avg_cost);
std::vector<Route> perform_2_opt_move(std::vector<Route>& solution, Move move, double avg_cost);
std::vector<Route> perform_2_opt_prim_move(std::vector<Route>& solution, Move move, double avg_cost);
std::vector<Route> perform_corss_tail_move(std::vector<Route>& solution, Move move, double avg_cost);