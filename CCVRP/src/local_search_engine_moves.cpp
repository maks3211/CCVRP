#include "local_search_engine_moves.h"
//brak pliku .cpp ze wzgledu na obecnosc template - wtedy sie stoduje chyba pliki tpp, no ale zrobione jest wszystko w jednym .h
//INTRA - jedna trasa
//INTER - dwie rozne trasy



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



bool update_best_k_moves(BestMoves& best, double gain, int k, Move& move)
{
	move.gain = gain; // przypisujemy zysk do ruchu

	//nie ma jeszcze zapisanych k ruchow 
	if (best.topK.size() < k)
	{
		best.topK.push_back(move);

		// sortujemy malej¹co (najlepszy na pocz¹tku)
		std::sort(best.topK.begin(), best.topK.end(),
			[](const Move& a, const Move& b) { return a.gain > b.gain; });

		// aktualizujemy worst
		best.worst_in_topK = best.topK.back().gain;

		return true;
	}
	//jest juz k najlepszych ruchow 
	if (gain > best.worst_in_topK)
	{
		best.topK.push_back(move);

		std::sort(best.topK.begin(), best.topK.end(),
			[](const Move& a, const Move& b) { return a.gain > b.gain; });

		// usuwamy najgorszy
		best.topK.pop_back();

		// aktualizujemy worst
		best.worst_in_topK = best.topK.back().gain;

		return true;
	}

	return false;
}

//wykorzystane w cross_tail
//calculate_crosstail_gain zwraca vector trzech ruchow - czyli dla obu odwroconych, i jednego z dwoch odwroconych

bool update_best_k_moves_from_vector(BestMoves& best, int k, std::vector<Move>& moves)
{
	bool improved = false;

	for (int i = 0; i < moves.size(); i++)
	{
		const Move& move = moves[i];


		if (best.topK.size() < k)
		{
			best.topK.push_back(move);
			improved = true;
		}
		else if (move.gain > best.worst_in_topK)
		{
			best.topK.push_back(move);
			improved = true;
		}
	}

	if (improved)
	{
		std::sort(best.topK.begin(), best.topK.end(),
			[](const Move& a, const Move& b) { return a.gain > b.gain; });

		// przyciecie do K
		if (best.topK.size() > k)
		{
			best.topK.resize(k);
		}

		best.worst_in_topK = best.topK.back().gain;
	}

	return improved;
}


//												PIERWSZY OPERATOR 
// 
// 
//usuwam jednego klienta i wstwiam go na kazda inna mozliwa pozycje we wszystkioch mozliwych trasach

BestMoves get_top_k_1_insertion_moves(std::vector<Route>& solution, int k)
{
	//std::cout << "	Rozpoczeto get_top_k_1_insertion_moves \n";
	//przechowuje wynik koncowy - k najlepszych ruchow
	BestMoves best_k_moves;


	Move move;

	double avg_cost = 0.0;
	for (auto& a : solution)
	{
		avg_cost += a.route_cost;
	}
	avg_cost /= solution.size();

	double gain = 0.0;
	//pozycje w petlach sa od 1 poniewaz nie wstwaimy na indeks 0 czyli w miejsce magazynu

	for (int r_from = 0; r_from < solution.size(); ++r_from) // przejscie przez kazda trase
	{
		const Route& route_from = solution[r_from];

		for (int pos = 1; pos < route_from.customers.size(); ++pos) // kazdy klient w trasie
		{
			const Node& client = route_from.customers[pos];
			for (int r_to = 0; r_to < solution.size(); ++r_to) // przejscie przez wszystkie trasy do ktorych bedziemy wstawiac
			{
				const Route& route_to = solution[r_to]; // trasa do ktorej sprawdzamy wstawienie klienta
				for (int insert_pos = 1; insert_pos <= route_to.customers.size(); ++insert_pos)//przejscie przez wszystkie pozycje
				{
					if (r_from == r_to && (insert_pos == pos || insert_pos == pos + 1)) // wstawianie na to samo miejsce - pomijamy
					{
						continue;
					}

					move = { r_from, pos, r_to, insert_pos,1, 0.0, client.demand, 0 };
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
	best_k_moves.L_h = 1;
	//std::cout << "	Zakonczono get_top_k_1_insertion_moves \n";
	return best_k_moves;
}

//both intra - one route  and inter two routes 
//zaminiamy miejscami dwoch klientow 
BestMoves get_top_k_1_1_exchange_moves(std::vector<Route>& solution, int k)
{
	//std::cout << "	Rozpoczeto get_top_k_1_1_exchange_moves \n";
	BestMoves best_k_moves;

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
				move = { r_from, pos, r_from, exchange_pos,2, 0.0, 0, 0 }; //wymiana klienta A z klientem B w tej samej trasie
				gain = calculate_gain_1_1_exchange_hybrid(solution, move, avg_cost);
				update_best_k_moves(best_k_moves, gain, k, move);
			}
		}
	}

	//inter - different routes

	for (int r_from = 0; r_from < solution.size() - 1; ++r_from)
	{
		const Route& route_from = solution[r_from];
		for (int pos = 1; pos < route_from.customers.size(); ++pos)
		{
			const Node& clientA = route_from.customers[pos];
			for (int to_route = r_from + 1; to_route < solution.size(); ++to_route)
			{
				const Route& route_to = solution[to_route]; // trasa z ktora nastepuje wymiana klienta clientA
				for (int b_client_pos = 1; b_client_pos < route_to.customers.size(); ++b_client_pos)
				{
					const Node& clientB = route_to.customers[b_client_pos]; //klient z ktorym nastepuje wymiana clientA <-> clientB
					move = { r_from, pos, to_route, b_client_pos, 2,0.0, clientA.demand, clientB.demand }; //wymiana klienta A z klientem B miedzy dwiema trasami
					calculate_gain_1_1_exchange_hybrid(solution, move, avg_cost);
					update_best_k_moves(best_k_moves, gain, k, move);
				}
			}
		}

	}
	best_k_moves.L_h = 2;
	//std::cout << "	Zakonczono get_top_k_1_1_exchange_moves \n";
	return best_k_moves;
}


//Usuniecie dwoch kolejnych klientow (sasiadow) z jednej trasy i umieszczenie ich w innym mijescu
// w tej samej lub innej trasie oraz w tej samej lub odwroconej kolejnosci
//both intra and inter

BestMoves get_top_k_2_insertion_moves(std::vector<Route>& solution, int k)
{
	//std::cout << "	Rozpoczeto get_top_k_2_insertion_moves \n";
	BestMoves best_k_moves;

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
					if (r_from == r_to && (insert_pos == pos || insert_pos == pos + 1 || insert_pos == pos + 2)) //wstawianie na to samo miejsce
						continue;


					move = { r_from, pos, r_to, insert_pos, 2,0.0, demand_from,0 };
					gain = calculate_gain_2_insertion_hybrid(solution, move, avg_cost); // ta sama kolejnosc klientow
					update_best_k_moves(best_k_moves, gain, k, move);

					move = { r_from, pos, r_to, insert_pos, 2,0.0, demand_from,0, 1 };
					gain = calculate_gain_2_insertion_hybrid(solution, move, avg_cost);
					update_best_k_moves(best_k_moves, gain, k, move);

				}
			}
		}
	}
	best_k_moves.L_h = 3;
	//std::cout << "	Zakonczono get_top_k_2_insertion_moves \n";
	return best_k_moves;
}


//Intra only - dzilanie w obrebie jednej trasy
//wybieram dwa rozne punkty na trasie i odwracam caly fragment miedzy nimi
BestMoves get_top_k_2_opt_moves(std::vector<Route>& solution, int k)
{
	//std::cout << "	Rozpoczeto get_top_k_2_opt_moves \n";
	BestMoves best_k_moves;

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
				move = { r_from, first_customer, r_from, range, range - first_customer, 0.0, 0,0,1 };
			
				gain = calculate_gain_2_opt_hybrid(solution, move, avg_cost);
				
				update_best_k_moves(best_k_moves, gain, k, move);
				
			}
		}
	
	}
	best_k_moves.L_h = 4;
	//std::cout << "	Zakonczono get_top_k_2_opt_moves \n";
	return best_k_moves;
}

//Inter only - dzialanie miedzy dwiema roznymi trasami, nie mozna przestawiac klientow w obrebie jednej trasy
//zamieniem ogony dwoch tras - dlugosc ogonow oraz miejsce ciecia moze byc pomiedzy trasami
BestMoves get_top_k_2_opt_prim_moves(std::vector<Route>& solution, int k)
{
	//std::cout << "	Rozpoczeto get_top_k_2_opt_prim_moves \n";
	BestMoves best_k_moves;

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
					move = { routeA, tail_A_start_index, routeB, tail_B_start_index, tail_A_length, 0.0,demand_A,demand_B, 0, tail_B_length };
					gain = calculate_gain_2_opt_prim_hybrid(solution, move, avg_cost);
					update_best_k_moves(best_k_moves, gain, k, move);
				}

			}
		}
	}
	best_k_moves.L_h = 5;
	//std::cout << "	Zakonczono get_top_k_2_opt_prim_moves \n";
	return best_k_moves;
}

//Inter only - dzialanie miedzy dwiema roznymi trasami, nie mozna przestawiac klientow w obrebie jednej trasy
//analogicznie do 2_opt* ale tutaj dodatkowo te przestawiane framgenty moga byc odwracane - czyli zamieniam ogony dwoch tras ale dodatkowo moge odwracac te ogony
//wiec w praktyce najpierw licze 2_opt* a potem dla tych samych punktow ciecia licze 2_opt* ale z odwracaniem fragmentow (rev/org, rev/rev, org/rev)
BestMoves get_top_k_cross_tail_moves(std::vector<Route>& solution, int k)
{
	//std::cout << "	Rozpoczeto get_top_k_cross_tail_moves \n";
	BestMoves best_k_moves;

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
					update_best_k_moves_from_vector(best_k_moves, k, moves);
				}

			}
		}
	}
	best_k_moves.L_h = 6;
	//std::cout << "	Zakonczono get_top_k_cross_tail_moves \n";
	return best_k_moves;
}


BestMoves L_h_local_serach(std::vector<Route>& solution, int h, int k)
{
	switch (h) {
	case 1:
		return get_top_k_1_insertion_moves(solution, k);
		break;
	case 2:
		return get_top_k_1_1_exchange_moves(solution, k);
		break;
	case 3:
		return get_top_k_2_insertion_moves(solution, k);
		break;
	case 4:
		return get_top_k_2_opt_moves(solution, k);
		break;
	case 5:
		return get_top_k_2_opt_prim_moves(solution, k);
		break;
	case 6:
		return get_top_k_cross_tail_moves(solution, k);
		break;
	default:
		BestMoves{};
	}

}


void check_fucking_capacity(std::vector<Route> przed, std::vector<Route> po, std::string nazwa)
{
	
	for (int i = 0; i < przed.size(); i++)
	{
		int org = przed[i].remaining_capacity;
		int cal = calculate_remaining_capacity(przed[i]);
		if (org != cal)
		{
			std::cout << "		" << nazwa << "\033[33m" << "\n    Wejscie perform_local_move Pozstala pojemnosc org: " << i << " " << org << " ,przeliczona: " << cal << "\033[0m" << std::endl;
		}
	}

	for (int i = 0; i < po.size(); i++)
	{
		int org = po[i].remaining_capacity;
		int cal = calculate_remaining_capacity(po[i]);
		if (org != cal)
		{
			std::cout <<"		" << nazwa << "\033[33m" << "\n    WYJSCIE perform_local_move Pozstala pojemnosc org : " << i << " " << org << ", przeliczona : " << cal << "\033[0m" << std::endl;
		}
	}
}

bool was_1_insertion = false;
bool was_1_1_exchange = false;
bool was_2_insertion = false;
bool was_2_opt_move = false;
bool was_2_opt_prim = false;
bool was_2_corss_tail = false;
std::vector<Route> perform_local_move(std::vector<Route>& solution, Move move, int structure, double avg_cost)
{
	std::vector<Route> my_solution = solution;
	switch (structure) {
	case 1:
		//std::cout << "	ROZPOCZETO perform_1_insertion_move \n";
		my_solution = perform_1_insertion_move(solution, move, avg_cost); // TU JEST BLAD POJEMNOSCI !!!! poprawiono 		
		return my_solution;
		//return perform_1_insertion_move(solution, move, avg_cost);
		break;
	case 2:
		//std::cout << "	ROZPOCZETO perform_1_1_exchange_move \n";
		//return perform_1_1_exchange_move(solution, move, avg_cost);	
		my_solution = perform_1_1_exchange_move(solution, move, avg_cost);
		//check_fucking_capacity(solution, my_solution, "perform_1_1_exchange_move");
		if (!was_1_1_exchange)
		{
			was_1_1_exchange = true;
			std::cout << "\033[32m" << "=============================WYWKONANO perform_1_1_exchange_move ============================" << "\033[0m" << std::endl;
		}
		return my_solution;
		break;
	case 3:
		//std::cout << "	ROZPOCZETO perform_2_insertion_move \n";
		//return perform_2_insertion_move(solution, move, avg_cost);
		my_solution = perform_2_insertion_move(solution, move, avg_cost); //TU JEST BLAD POJEMNOSCI !!!!
		//check_fucking_capacity(solution, my_solution, "perform_2_insertion_move");
		if (!was_2_insertion)
		{
			was_2_insertion = true;
			std::cout << "\033[32m" << "=============================WYWKONANO perform_2_insertion_move============================" << "\033[0m" << std::endl;
		}
		return my_solution;
		break;
	case 4:
		//std::cout << "	ROZPOCZETO perform_2_opt_move \n";
		//return perform_2_opt_move(solution, move, avg_cost); //tez zwraca blad	
		my_solution = perform_2_opt_move(solution, move, avg_cost);
		//check_fucking_capacity(solution, my_solution, "perform_2_opt_move");
		if (!was_2_opt_move)
		{
			was_2_opt_move = true;
			std::cout << "\033[32m" << "=============================WYWKONANO perform_2_opt_move - operator wewnatrz jednej trasy============================" << "\033[0m" << std::endl;
		}
		return my_solution;
		break;
	case 5:
		//std::cout << "	ROZPOCZETO perform_2_opt_prim_move \n";
		//return perform_2_opt_prim_move(solution, move, avg_cost);
		my_solution = perform_2_opt_prim_move(solution, move, avg_cost);
		//check_fucking_capacity(solution, my_solution, "perform_2_opt_prim_move");
		if (!was_2_opt_prim)
		{
			was_2_opt_prim = true;
			std::cout << "\033[32m" << "=============================WYWKONANO perform_2_opt_prim_move============================" << "\033[0m" << std::endl;
		}
		return my_solution;
		break;
	case 6:
		//std::cout << "	ROZPOCZETO perform_corss_tail_move \n";
		//return perform_corss_tail_move(solution, move, avg_cost);
		my_solution = perform_corss_tail_move(solution, move, avg_cost);
		//check_fucking_capacity(solution, my_solution, "perform_corss_tail_move");
		if (!was_2_corss_tail)
		{
			was_2_corss_tail = true;
			std::cout << "\033[32m" << "=============================WYWKONANO perform_corss_tail_move============================" << "\033[0m" << std::endl;
		}
		return my_solution;
		break;
	}
}

//realzuje ruch 
//usun jednego klienta i wstaw na inna pozycje
std::vector<Route> perform_1_insertion_move(std::vector<Route>& solution, Move move, double avg_cost)
{
	std::vector<Route> my_solution = solution;

	//klient ktorego przstawiam
	Node changing_customer = solution[move.from_route].customers[move.from_pos];

	bool same_route = move.from_route == move.to_route;

	//przestawiamy klienta w obrebie tej samej trasy
	if (same_route)
	{
		int index = move.to_pos;
		if (move.from_pos < move.to_pos)
		{
			index--;
		}
		//usuniecie klienta
		
		my_solution[move.from_route].customers.erase(my_solution[move.from_route].customers.begin() + move.from_pos); 

		//wstawienie klienta
		my_solution[move.from_route].customers.insert(my_solution[move.from_route].customers.begin() +index, changing_customer);
	
		my_solution[move.from_route].recalculate_all();

	}
	else //przenisienie z trasy A do trasy B
	{
		my_solution[move.from_route].remaining_capacity += changing_customer.demand; //dodanie wolnego miejsca usuwanego klienta
		my_solution[move.from_route].customers.erase(my_solution[move.from_route].customers.begin() + move.from_pos);
		
		my_solution[move.to_route].remaining_capacity -= changing_customer.demand; // zabranie wolnego miejsca w trasie do ktorej dodaje klienta
		my_solution[move.to_route].customers.insert(my_solution[move.to_route].customers.begin() + move.to_pos, changing_customer);
		

		//obliczenie wykorzystanej pojemnosci trasy z kotrej usuwam
		int new_used_capacity = my_solution[move.from_route].initial_capacity - my_solution[move.from_route].remaining_capacity;

		my_solution[move.from_route].penatly_eta = calculate_penalty_hybrid(my_solution[move.from_route].initial_capacity, new_used_capacity, avg_cost);

		new_used_capacity = my_solution[move.to_route].initial_capacity - my_solution[move.to_route].remaining_capacity;
		my_solution[move.to_route].penatly_eta = calculate_penalty_hybrid(my_solution[move.from_route].initial_capacity, new_used_capacity, avg_cost);

		my_solution[move.from_route].recalculate_all();
		my_solution[move.to_route].recalculate_all();
	}
	
	return my_solution;
}


//zamiana miejscami dwoch klientow
std::vector<Route> perform_1_1_exchange_move(std::vector<Route>& solution, Move move, double avg_cost)
{
	std::vector<Route> my_solution = solution;
	Node client_A = solution[move.from_route].customers[move.from_pos];
	Node client_B = solution[move.to_route].customers[move.to_pos];

	std::swap(my_solution[move.from_route].customers[move.from_pos], my_solution[move.to_route].customers[move.to_pos]);

	//sa rozne trasy trzeba przeliczyc pojemnosc
	if (move.from_route != move.to_route)
	{
		my_solution[move.from_route].remaining_capacity += client_A.demand; //dodajemy pojemnosci klientow ktorych wyciagamy z tras
		my_solution[move.to_route].remaining_capacity += client_B.demand;

		my_solution[move.from_route].remaining_capacity -= client_B.demand; // odejmujemy pojemnosci klientow ktrzy sa dodani do tras
		my_solution[move.to_route].remaining_capacity -= client_A.demand;


		int new_used_capacity = my_solution[move.from_route].initial_capacity - my_solution[move.from_route].remaining_capacity;
		my_solution[move.from_route].penatly_eta = calculate_penalty_hybrid(my_solution[move.from_route].initial_capacity, new_used_capacity, avg_cost);
		new_used_capacity = my_solution[move.to_route].initial_capacity - my_solution[move.to_route].remaining_capacity;
		my_solution[move.to_route].penatly_eta = calculate_penalty_hybrid(my_solution[move.to_route].initial_capacity, new_used_capacity, avg_cost);
		my_solution[move.to_route].recalculate_all();

		my_solution[move.from_route].recalculate_all();
		my_solution[move.to_route].recalculate_all();
	}

	//te same trasy - przeliczenie kosztu tylko from_route bo to to samo co to_route
	else	
	{
		my_solution[move.from_route].recalculate_all();
	}
	
	return my_solution;
}

//usuniecie dwoch sasiadow z trasy i wstawienie i inna pozycje (org lub odwrocone)
std::vector<Route> perform_2_insertion_move(std::vector<Route>& solution, Move move, double avg_cost)
{
	//przestawienie w obrebie tej samej trasy   wstawienie na miejsce drugiego z pary   wstawienei na to samo miejsce bez odwracania 
	if (move.from_route == move.to_route && (move.to_pos - move.from_pos == 1 || (move.to_pos == move.from_pos && move.orientation == 1)))    // przestawienie na to samo miejsce lub przestawienie na miejsce drugiego elemntu z pary
	{
	
		return solution;
	}
	std::vector<Route> my_solution = solution;

	auto& from_vec = my_solution[move.from_route].customers;
	auto& to_vec = my_solution[move.to_route].customers;

	auto first = from_vec.begin() + move.from_pos;
	auto last = from_vec.begin() + move.from_pos + 2;
	std::vector<Node> temp(first, last);
	if (move.orientation == 1) {
		std::reverse(temp.begin(), temp.end());
	}

	
	from_vec.erase(first, last);
	int insertion_index = move.to_pos;
	//w tej samej trasie
	if (move.from_route == move.to_route)
	{
		if (move.to_pos > move.from_pos)
		{
			insertion_index -= 2;
			if (insertion_index <= 0)
			{
				return solution;
			}
		}
		to_vec.insert(to_vec.begin() + insertion_index, std::make_move_iterator(temp.begin()),
			std::make_move_iterator(temp.end()));
		
		my_solution[move.from_route].recalculate_all(); // nie trzeba przeliczc kosztow
		
		return my_solution;

	}

	//w obrebie dwoch tras 
	to_vec.insert(to_vec.begin() + insertion_index, std::make_move_iterator(temp.begin()),
		std::make_move_iterator(temp.end()));


	//przeliczenie kosztow
	my_solution[move.from_route].remaining_capacity += move.moved_capacity_from;
	my_solution[move.to_route].remaining_capacity -= move.moved_capacity_from;

	int new_used_capacity = my_solution[move.from_route].initial_capacity - my_solution[move.from_route].remaining_capacity;
	my_solution[move.from_route].penatly_eta = calculate_penalty_hybrid(my_solution[move.from_route].initial_capacity, new_used_capacity, avg_cost);

	new_used_capacity = my_solution[move.to_route].initial_capacity - my_solution[move.to_route].remaining_capacity;
	my_solution[move.to_route].penatly_eta = calculate_penalty_hybrid(my_solution[move.to_route].initial_capacity, new_used_capacity, avg_cost);

	my_solution[move.from_route].recalculate_all();
	my_solution[move.to_route].recalculate_all();

	return my_solution;
}

//odwrocenie fragmentu calej trasy miedzy wybranimiy punktami
std::vector<Route> perform_2_opt_move(std::vector<Route>& solution, Move move, double avg_cost)
{
	std::vector<Route> my_solution = solution;
	int from_index = move.from_pos;
	int range = move.to_pos;
	Route& r = my_solution[move.from_route];
	//tu byl juz raz wykryty blad i zostal poprawiony
	//std::reverse(r.customers.begin() + from_index, r.customers.begin() + from_index + range); //move.to_pos wskazuje na ostatni indeks ktory ma byc zamienony a nie ile elmtow zamienic- to wskazuje move.number_of_moved_clients
	std::reverse(r.customers.begin() + from_index, r.customers.begin() +  range);

	r.recalculate_all();
	
	return my_solution;
}


//zamiana ognow dwoch tras
std::vector<Route> perform_2_opt_prim_move(std::vector<Route>& solution, Move move, double avg_cost)
{
	//move = { routeA, tail_A_start_index, routeB, tail_B_start_index, tail_A_length, 0.0,demand_A,demand_B, 0, tail_B_length
	std::vector<Route> my_solution = solution;


	Route& vec_A = my_solution[move.from_route];
	Route& vec_B = my_solution[move.to_route];

	//zapisanie ogonow
	std::vector<Node> tail_A(vec_A.customers.begin() + move.from_pos, vec_A.customers.end());
	std::vector<Node> tail_B(vec_B.customers.begin() + move.to_pos, vec_B.customers.end());


	vec_A.customers.erase(vec_A.customers.begin() + move.from_pos, vec_A.customers.end());
	vec_B.customers.erase(vec_B.customers.begin() + move.to_pos, vec_B.customers.end());


	vec_A.customers.insert(vec_A.customers.end(), std::make_move_iterator(tail_B.begin()), std::make_move_iterator(tail_B.end()));
	vec_B.customers.insert(vec_B.customers.end(), std::make_move_iterator(tail_A.begin()), std::make_move_iterator(tail_A.end()));

	//przeliczenie kosztow


	vec_A.remaining_capacity += move.moved_capacity_from;
	vec_A.remaining_capacity -= move.moved_capacity_to;

	int new_used_capacity = vec_A.initial_capacity - solution[move.from_route].remaining_capacity;
	vec_A.penatly_eta = calculate_penalty_hybrid(my_solution[move.from_route].initial_capacity, new_used_capacity, avg_cost);

	vec_A.remaining_capacity += move.moved_capacity_to;
	vec_A.remaining_capacity -= move.moved_capacity_from;
	new_used_capacity = vec_B.initial_capacity - solution[move.to_route].remaining_capacity;
	vec_B.penatly_eta = calculate_penalty_hybrid(my_solution[move.to_route].initial_capacity, new_used_capacity, avg_cost);


	vec_A.recalculate_all();
	vec_B.recalculate_all();
	return my_solution;
}

//analogicznie jak wyzej + odwrocenie kolejnosci ogona
std::vector<Route> perform_corss_tail_move(std::vector<Route>& solution, Move move, double avg_cost)
{
	std::vector<Route> my_solution = solution;


	Route& vec_A = my_solution[move.from_route];
	Route& vec_B = my_solution[move.to_route];

	//zapisanie ogonow
	std::vector<Node> tail_A(vec_A.customers.begin() + move.from_pos, vec_A.customers.end());
	std::vector<Node> tail_B(vec_B.customers.begin() + move.to_pos, vec_B.customers.end());

	//odwrocenie obu czesci
	if (move.orientation == 1)
	{
		std::reverse(tail_A.begin(), tail_A.end());
		std::reverse(tail_B.begin(), tail_B.end());
	}
	//odwrocenie tylko A
	else if (move.orientation == 2)
	{
		std::reverse(tail_A.begin(), tail_A.end());
	}
	//odwrocenie B
	else if (move.orientation == 3)
	{
		std::reverse(tail_B.begin(), tail_B.end());
	}
	//jezeli jest == 4 to nie odwracam 

	vec_A.customers.erase(vec_A.customers.begin() + move.from_pos, vec_A.customers.end());
	vec_B.customers.erase(vec_B.customers.begin() + move.to_pos, vec_B.customers.end());


	vec_A.customers.insert(vec_A.customers.end(), std::make_move_iterator(tail_B.begin()), std::make_move_iterator(tail_B.end()));
	vec_B.customers.insert(vec_B.customers.end(), std::make_move_iterator(tail_A.begin()), std::make_move_iterator(tail_A.end()));

	//przeliczenie kosztow
	vec_A.remaining_capacity += move.moved_capacity_from;
	vec_A.remaining_capacity -= move.moved_capacity_to;

	int new_used_capacity = vec_A.initial_capacity - solution[move.from_route].remaining_capacity;
	vec_A.penatly_eta = calculate_penalty_hybrid(my_solution[move.from_route].initial_capacity, new_used_capacity, avg_cost);

	vec_B.remaining_capacity += move.moved_capacity_to;
	vec_B.remaining_capacity -= move.moved_capacity_from;

	new_used_capacity = vec_B.initial_capacity - solution[move.to_route].remaining_capacity;
	vec_B.penatly_eta = calculate_penalty_hybrid(my_solution[move.to_route].initial_capacity, new_used_capacity, avg_cost);

	vec_A.recalculate_all();
	vec_B.recalculate_all();
	
	return my_solution;
}