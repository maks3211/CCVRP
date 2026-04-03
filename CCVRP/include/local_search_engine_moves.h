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

//wykorzystane w cross_tail
//calculate_crosstail_gain zwraca vector trzech ruchow - czyli dla obu odwroconych, i jednego z dwoch odwroconych

bool update_best_k_moves_from_vector(BestMoves& best, int k, std::vector<Move>& moves)
{
	for(int i = 0; i < moves.size();i++ )
	{
		if (moves[i].gain > best.worst_in_topK) // otrzymany wynik jest lepszy od najgorszego z dotychczasowych najlepszych ruchow
		{
			best.topK.push_back(moves[i]); // dodajemy nowy ruch do listy najlepszych
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
	
}

//usuwam jednego klienta i wstwiam go na kazda inna mozliwa pozycje we wszystkioch mozliwych trasach
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
					gain = calculate_gain_1_insertion_hybrid(solution, move, avg_cost);  //
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
				gain = calculate_gain_1_1_exchange_hybrid(solution, move, avg_cost);  
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
					calculate_gain_1_1_exchange_hybrid(solution, move, avg_cost);
					update_best_k_moves(best_k_moves, gain, k, move);
				}
			}
		}
		
	}
	return best_k_moves;
}


//Usuniecie dwoch kolejnych klientow (sasiadow) z jednej trasy i umieszczenie ich w innym mijescu
// w tej samej lub innej trasie oraz w tej samej lub odwroconej kolejnosci
//both intra and inter
template<typename GainFunction>
std::vector<Move> get_top_k_2_insertion_moves(std::vector<Route>& solution, int k, GainFunction gain_func)
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

	
	for (int r_from = 0; r_from < solution.size(); ++r_from) // przejscie przez kazda trase 
	{
		const Route& route_from = solution[r_from];
		
		for (int pos = 1; pos < route_from.customers.size() - 1; ++pos) // przejscie przez wszystkie pary sasiadujacych klientow w trasie
		{
			int demand_from = route_from.customers[pos].demand + route_from.customers[pos + 1].demand; // suma demandu dwoch klientow do przeniesienia
			for (int r_to = 0; r_to < solution.size(); ++r_to)
			{
				const Route& route_to = solution[r_to];
				for (int insert_pos = 1; insert_pos <= route_to.customers.size(); ++insert_pos)
				{
					if (r_from == r_to && (insert_pos == pos || insert_pos == pos + 1 || insert_pos == pos + 2 )) //wstawianie na to samo miejsce
						continue;
					

					move = { r_from, pos, r_to, insert_pos, 2,0.0, demand_from,0};
					gain = calculate_gain_2_insertion_hybrid(solution, move, avg_cost); // ta sama kolejnosc klientow
					update_best_k_moves(best_k_moves, gain, k, move);
				
					move = { r_from, pos, r_to, insert_pos, 2,0.0, demand_from,0, 1};
					gain = calculate_gain_2_insertion_hybrid(solution, move, avg_cost);
					update_best_k_moves(best_k_moves, gain, k, move);

				}
			}
		}
	}
	return best_k_moves;
}


//Intra only - dzilanie w obrebie jednej trasy
//wybieram dwa rozne punkty na trasie i odwracam caly fragment miedzy nimi
template<typename GainFunction>
std::vector<Move> get_top_k_2_opt_moves(std::vector<Route>& solution, int k, GainFunction gain_func)
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

	for (int r_from = 0; r_from < solution.size(); ++r_from) // przejscie przez kazda trase 
	{
		const Route& route_from = solution[r_from];

		for (int first_customer = 1; first_customer < route_from.customers.size() - 1; ++first_customer) // -1 poniewaz moge zamienic sie z ostatnim
		{
			for (int range = first_customer + 1; range < route_from.customers.size(); ++range)
			{
				move = {r_from, first_customer, r_from, range, range-first_customer, 0.0, 0,0,1};
				gain = calculate_gain_2_opt_hybrid(solution, move, avg_cost);
				update_best_k_moves(best_k_moves, gain, k, move);
			}
		}
	}
	return best_k_moves;
}

//Inter only - dzialanie miedzy dwiema roznymi trasami, nie mozna przestawiac klientow w obrebie jednej trasy
//zamieniem ogony dwoch tras - dlugosc ogonow oraz miejsce ciecia moze byc pomiedzy trasami
template<typename GainFunction>
std::vector<Move> get_top_k_2_opt_prim_moves(std::vector<Route>& solution, int k, GainFunction gain_func)
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
	for (int routeA = 0; routeA < solution.size() - 1; ++routeA) // pierwsza trasa z ktorej bierzemy zbior klientow
	{
		const Route& route_A = solution[routeA];
		if (route_A.customers.size() < 2)
		{
			continue;
		}
		//przejscie przez wszystkie mozliwe poczatki ogona
		for (int tail_A_start_index = 1; tail_A_start_index < route_A.customers.size(); ++ tail_A_start_index) 
		{
			int tail_A_length = route_A.customers.size() - tail_A_start_index;
			int demand_A = 0;
			for (int i = tail_A_start_index; i < route_A.customers.size(); ++i)
			{
				demand_A += route_A.customers[i].demand;
			}

			//wymiana ogona A z wszystkimi mozliwimi wymianami z pozostalych tras
			for (int routeB = routeA + 1; routeB < solution.size(); ++routeB)
			{
				const Route& route_B = solution[routeB];
				if (route_B.customers.size() < 2)
				{
					continue;
				}
				for (int tail_B_start_index = 1; tail_B_start_index < route_B.customers.size(); ++tail_B_start_index)
				{
					if (tail_A_start_index == 1 && tail_B_start_index == 1) // czyli nie wymieniam ogonow jezeli oba zaczynaja sie od pierwszego elemntu- wtedy wymiana calej trasy 
					{
						continue;
					}
					int tail_B_length = route_B.customers.size() - tail_B_start_index;
					int demand_B = 0;
					for (int i = tail_B_start_index; i < route_B.customers.size(); ++i)
					{
						demand_B += route_B.customers[i].demand;
					}
					move = {routeA, tail_A_start_index, routeB, tail_B_start_index, tail_A_length, 0.0,demand_A,demand_B, 0, tail_B_length};
					gain = calculate_gain_2_opt_prim_hybrid(solution, move, avg_cost);
					update_best_k_moves(best_k_moves, gain, k, move);
				}

			}
		}
	}
	return best_k_moves;
}

//Inter only - dzialanie miedzy dwiema roznymi trasami, nie mozna przestawiac klientow w obrebie jednej trasy
//analogicznie do 2_opt* ale tutaj dodatkowo te przestawiane framgenty moga byc odwracane - czyli zamieniam ogony dwoch tras ale dodatkowo moge odwracac te ogony
//wiec w praktyce najpierw licze 2_opt* a potem dla tych samych punktow ciecia licze 2_opt* ale z odwracaniem fragmentow (rev/org, rev/rev, org/rev)
template<typename GainFunction>
std::vector<Move> get_top_k_cross_tail_moves(std::vector<Route>& solution, int k, GainFunction gain_func)
{
	BestMoves best_k_moves;
	best_k_moves.topK.reserve(k + 1);
	Move move;

	std::vector<Move> moves;


	double avg_cost = 0.0;
	for (auto& a : solution)
	{
		avg_cost += a.route_cost;
	}
	avg_cost /= solution.size();

	double gain = 0.0;
	for (int routeA = 0; routeA < solution.size() - 1; ++routeA) // pierwsza trasa z ktorej bierzemy zbior klientow
	{
		const Route& route_A = solution[routeA];
		if (route_A.customers.size() < 2)
		{
			continue;
		}
		//przejscie przez wszystkie mozliwe poczatki ogona
		for (int tail_A_start_index = 1; tail_A_start_index < route_A.customers.size(); ++tail_A_start_index)
		{
			int tail_A_length = route_A.customers.size() - tail_A_start_index;
			int demand_A = 0;
			for (int i = tail_A_start_index; i < route_A.customers.size(); ++i)
			{
				demand_A += route_A.customers[i].demand;
			}

			//wymiana ogona A z wszystkimi mozliwimi wymianami z pozostalych tras
			for (int routeB = routeA + 1; routeB < solution.size(); ++routeB)
			{
				const Route& route_B = solution[routeB];
				if (route_B.customers.size() < 2)
				{
					continue;
				}
				for (int tail_B_start_index = 1; tail_B_start_index < route_B.customers.size(); ++tail_B_start_index)
				{
					if (tail_A_start_index == 1 && tail_B_start_index == 1) // czyli nie wymieniam ogonow jezeli oba zaczynaja sie od pierwszego elemntu- wtedy wymiana calej trasy 
					{
						continue;
					}
					int tail_B_length = route_B.customers.size() - tail_B_start_index;
					int demand_B = 0;
					for (int i = tail_B_start_index; i < route_B.customers.size(); ++i)
					{
						demand_B += route_B.customers[i].demand;
					}

					//Wszystko jest identyczne tylko tutaj ogony moga miec odwrocona kolejnosc, no i jest inna funkcja do gain-a 
					// 1 - oba ogony odwrocone  2- tylko A odwrocone  3- tylko B odwrocone
					//Dokladnie to samo co w 2-opt - tylko inna funkcja obliczajaca gain - ona w srodku bedzie liczyc 
					move = { routeA, tail_A_start_index, routeB, tail_B_start_index, tail_A_length, 0.0,demand_A,demand_B, 0, tail_B_length };
					moves = calculate_gain_cross_tail_hybrid(solution, move, avg_cost);
					update_best_k_moves_from_vector(best_k_moves, gain, k, move);
					

				}

			}
		}
	}
	return best_k_moves;
}
