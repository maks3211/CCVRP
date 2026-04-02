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
    double worst_in_topK = -1e9; //informacja ile wynosi najgorzszy z K wynikow
};


// 
//
//=============================== IMPLENTACJA LOCAL SERACH ENGINE DLA Hybryy - 198 ===============================
// 
// 
 

//oznaczone jako Lh

//uzycie - get_top_k_1insertion_moves(solution, k, g) - zwraca k najlepszych ruchow 1-insertions dla danej solucji i funkcji zysku g
//gdzie g to funkcja double g(sol 1, sol 2)...

//both intra and inter
//Trzy to parmetr z artykulu (strona 18 - we define K = 3 for kth best improvement)
//przeniosimy klienta na kazda inna mozliwa pozycje w obrebie wszystkich tras (oraz w tej samej) -
//czyli w praktyce sprawdzam wszystkie mozliwe pozycje w calym rozwiazaniu)
//czyli to bedzie korzystac z funckji - w momencie wywolywania tego ruchu to musze przekazac calculate_gain_1_insertion_gain_hybrid 



bool update_best_k_moves(BestMoves& best, double gain, int k, Move& move)
{
	if (gain > best.worst_in_topK) // otrzymany wynik jest lepszy od najgorszego z dotychczasowych najlepszych ruchow
	{
		best.topK.push_back(move); // dodajemy nowy ruch do listy najlepszych
		move.gain = gain; // przypisujemy zysk do ruchu
		if (best.topK.size() > k)
		{
			std::sort(best.topK.begin(), best.topK.end(), [](const Move& a, const Move& b) { return a.gain > b.gain; });
			best.topK.pop_back();
			best.worst_in_topK = best.topK.back().gain; // aktualizujemy najgorszy wynik w top K
			//czyli best_k_moves[0] to najelpszy ruch wzor 14
		}
		return true;
	}
	return false; 
}



template<typename GainFunction>
BestMoves get_top_k_1_insertion_moves(std::vector<Route>& solution, int k = 3, GainFunction gain_func)
{
	//przechowuje wynik koncowy - k najlepszych ruchow
	BestMoves best_k_moves;
	best_k_moves.topK.reserve(k + 1);
	
	Move move; 
	
	double avg_cost = 0.0;
	for (auto& a : solution)
	{
		avg_cost += a.route_cost;
	}
	avg_cost /= solution.size();

	double gain = 0.0;
	//pozycje w petlach sa od 1 poniewaz nie wstwaimy na indeks 0 czyli w miejsce magazynu

	for (int r_from = 0 ; r_from < solution.size(); ++r_from) // przejscie przez kazda trase
	{
		const Route& route_from = solution[r_from];

		for (int pos = 1; pos < route_from.customers.size(); ++pos) // kazdy klient w trasie
		{
			const Node& client = route_from.customers[pos];
			for (int r_to = 0; r_to < solution.size(); ++r_to) // przejscie przez wszystkie trasy do ktorych bedziemy wstawiac
			{
				const Route& route_to = solution.routes[r_to]; // trasa do ktorej sprawdzamy wstawienie klienta
				for (int insert_pos = 1; insert_pos =< route_to.customers.size(); ++insert_pos)//przejscie przez wszystkie pozycje
				{
					if (r_from == r_to && (insert_pos == pos || insert_pos == pos + 1)) // wstawianie na to samo miejsce - pomijamy
					{
						continue;
					}
					 
					move = { r_from, pos, r_to, insert_pos,1, 0.0, client.demand, 0.0};
					gain = calculate_gain_1_insertion_gain_hybrid(solution, move, avg_cost);  //
					update_best_k_moves(best_k_moves, gain, k, move);
					//if (gain > best_k_moves.worst_in_topK) // otrzymany wynik jest lepszy od najgorszego z dotychczasowych najlepszych ruchow
					//{
					//	best_k_moves.topK.push_back(move); // dodajemy nowy ruch do listy najlepszych
					//	if (best_k_moves.topK.size() > k)
					//	{
					//		std::sort(best_k_moves.topK.begin(), best_k_moves.topK.end(), [](const Move& a, const Move& b) { return a.gain > b.gain; });
					//		best_k_moves.topK.pop_back();
					//		best_k_moves.worst_in_topK = best_k_moves.topK.back().gain; // aktualizujemy najgorszy wynik w top K
					//		//czyli best_k_moves[0] to najelpszy ruch wzor 14
					//	}
					//}
				}
			}
		}
	}

	return best_k_moves;
}

//both intra - one route  and inter two routes 
//zaminiamy miejscami dwoch klientow 
template<typename GainFunction>
std::vector<Move> get_top_k_1_1_exchange_moves(std::vector<Route>& solution, int k = 3, GainFunction gain_func)
{
	BestMoves best_k_moves;
	best_k_moves.topK.reserve(k + 1);
	Move move;

	double avg_cost = 0.0;
	for (auto& a : solution)
	{
		avg_cost += a.route_cost;
	}
	avg_cost /= solution.size();

	double gain = 0.0;
	//intra - same route 
	for (int r_from = 0; r_from < solution.size(); ++r_from) // przejscie przez kazda trase 
	{
		const Route& route_from = solution[r_from];
		for (int pos = 1; pos < route_from.customers.size() - 1; ++pos)
		{
			//const Node& clientA = route_from.customers[pos];
			for (int exchange_pos = pos + 1; exchange_pos < route_from.customers.size(); ++exchange_pos)
			{
				//const Node& clientB = route_from.customers[exchange_pos];
				move = { r_from, pos, r_from, exchange_pos,2, 0.0, 0.0, 0.0 }; //wymiana klienta A z klientem B w tej samej trasie
				gain = calculate_gain_1_1_exchange_gain_hybrid(solution, move, avg_cost);  
				update_best_k_moves(best_k_moves, gain, k, move);
			}
		}
	}

	//inter - different routes

	for (int r_from = 0; r_from < solution.size() - 1 ; ++r_from)
	{
		const Route& route_from = solution[r_from];
		for (int pos = 1; pos < route_from.size(); ++pos)
		{
			const Node& clientA = route_from.customers[pos];
			for (int to_route = r_from + 1; to_route < solution.size(); ++to_route)
			{
				const Route& route_to = solution[to_route]; // trasa z ktora nastepuje wymiana klienta clientA
				for (int b_client_pos = 1; b_client_pos < route_to.size(); ++b_client_pos)
				{
					const Node& clientB = route_to.customers[b_client_pos]; //klient z ktorym nastepuje wymiana clientA <-> clientB
					move = { r_from, pos, to_route, b_client_pos, 2,0.0, clientA.demand, clientB.demand }; //wymiana klienta A z klientem B miedzy dwiema trasami
					calculate_gain_1_1_exchange_gain_hybrid(solution, move, avg_cost);
					update_best_k_moves(best_k_moves, gain, k, move);
				}
			}
		}
		
	}
	return best_k_moves;
}

//both intra and inter
template<typename GainFunction>
std::vector<Move> get_top_k_2_insertion_moves(std::vector<Route>& solution, int k, GainFunction gain_func)
{

}


//Intra only - dzilanie w obrebie jednej trasy
template<typename GainFunction>
std::vector<Move> get_top_k_2_opt_moves(std::vector<Route>& solution, int k, GainFunction gain_func)
{

}

//Inter only - dzialanie miedzy dwiema roznymi trasami, nie mozna przestawiac klientow w obrebie jednej trasy
template<typename GainFunction>
std::vector<Move> get_top_k_2_opt_prim_moves(std::vector<Route>& solution, int k, GainFunction gain_func)
{

}

//Inter only - dzialanie miedzy dwiema roznymi trasami, nie mozna przestawiac klientow w obrebie jednej trasy
template<typename GainFunction>
std::vector<Move> get_top_k_cross_tail_moves(std::vector<Route>& solution, int k, GainFunction gain_func)
{

}
