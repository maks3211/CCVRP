#include "lns_diversification_strategy.h"



std::vector<Route> lns_diversification(std::vector<Route>& solution, int lambda, double avg_route_cost, int total_customers)
{
	std::vector<Route> modified_solution = solution; 
	std::vector<clientRatioInfo> to_remove;
	std::unordered_set<int> unique_routes_remove; //zaweira infomracje z ktorych tras cos usunieto
	//1. wybor losowego operatora usuwania
	int method = random_int_from_to(1, 4);
	
	switch (method) {
	case 1:
		unique_routes_remove.clear();
		to_remove = random_removal(modified_solution, lambda); //to_ remove zawiera infomracje o tym jakich klienow usunac
		sort_descending(to_remove); // po sortowaniu jest  {B,2}, {B,0}, {A,9}, {A,3}, {A,1}  
		//sortowanie malajeco - moge usuwac od konca bez ryzuka prolbmeow z indeksami
		//usuwanie klientow
		//DO DEBUGOWANIA WYPISYWANIE LISTY KLIENTOW DO USUNIECIA
		/*std::cout << "Rozmiar: " << to_remove.size();
		for (int zdih = 0; zdih < to_remove.size(); ++zdih)
		{
			std::cout << " route idx = " << to_remove[zdih].route_index << " to_remove[" << zdih << "].client_index = " << to_remove[zdih].client_index << "\n";
		}*/
		for (int i = 0; i < to_remove.size(); ++i)
		{
			
			//sortowanie malejaco- moge usuwac od tylu 	
	
			unique_routes_remove.insert(to_remove[i].route_index);
		}
		break;

	case 2:
		unique_routes_remove.clear();
		to_remove = worst_removal(modified_solution, avg_route_cost, lambda); 
		sort_descending(to_remove);
		//usuwanie klientow
		for (int i = 0; i < to_remove.size(); ++i)
		{

		
			unique_routes_remove.insert(to_remove[i].route_index);
		}
		break;

	case 3:
		unique_routes_remove.clear();
		to_remove = worst_distance_removal(modified_solution, lambda);

		sort_descending(to_remove); 
		

		for (int i = 0; i < to_remove.size(); ++i)
		{	
			
			unique_routes_remove.insert(to_remove[i].route_index);
		}
		break;

	case 4:
		unique_routes_remove.clear();
		to_remove = confilcting_sector_removal(modified_solution, lambda);
		sort_descending(to_remove);
	
		for (int i = 0; i < to_remove.size(); ++i)
		{
		
			unique_routes_remove.insert(to_remove[i].route_index);
		}
		break;
	}
	
	double new_penalty_eta = 0.0;
	//przeliczenie tras z kotrych cos usunieto
	for (int route_idx : unique_routes_remove)
	{
	
		int new_demand = modified_solution[route_idx].initial_capacity - modified_solution[route_idx].remaining_capacity;
		new_penalty_eta = calculate_penalty_hybrid(modified_solution[route_idx].initial_capacity, new_demand, avg_route_cost);
		modified_solution[route_idx].penatly_eta = new_penalty_eta;
		modified_solution[route_idx].recalculate_all();
	}

	//4. wybor losowego operatora dodawania
	method = random_int_from_to(1, 2);
	

	Node new_customer;
	switch (method) 
	{
	case 1:
		clientRatioInfo to_add;
		for (int i = 0; i < to_remove.size(); ++i) //przejscie przez kazdego usunietego klienta aby go wstawic na nowa pozycje
		{
			int route_index = to_remove[i].route_index;
			int client_index = to_remove[i].client_index;
			new_customer = solution[route_index].customers[client_index];
			to_add = basic_greedy_insertion(modified_solution, new_customer, avg_route_cost);
		
			if (to_add.client_index == -1) // nie udalo sie nalezc miejsca - przeprowac perturbacje i odrazu aktualzacja calego rozwiazania
			{
			
				HybridAvnsLns::perform_perturbation(modified_solution, solution[route_index].customers[client_index], total_customers); 
				
				for (auto r : modified_solution) //przeliczam wszystkie trasy bo nie mam informacji o tym ktore zostaly zmienione
				{
					r.recalculate_all();	
				}
			
			}
			else // udalo sie znalezc pozycje dla klienta za pomoca basic_greed_insertion - mozna wstawic 
			{
				int insertion_route_index = to_add.route_index; //wstaw do tej trasy
				int insertion_client_index = to_add.client_index; //wstaw na ten indeks
				
				modified_solution[insertion_route_index].add_customer_at_index(new_customer,insertion_client_index,0.0,true);
				modified_solution[insertion_route_index].calculate_arrival_times();
				modified_solution[insertion_route_index].penatly_eta = to_add.ratio; // w ratio zapisuje delta_eta = calculate_penalty_hybrid(..)
			}	
		}
		break;
	case 2:
		clientInfo to_adds;
		//wektor wszystkich klientow ktorych musze wstawic 
		std::vector<Node> new_customers;
		
		for (int i = 0; i < to_remove.size(); ++i)
		{ //umieszczenie wszystkich usunietych klientow
			int route_index = to_remove[i].route_index;
			int client_index = to_remove[i].client_index;
			new_customers.push_back(solution[route_index].customers[client_index]);
		}

		for (int i = 0; i < to_remove.size(); ++i) //przejscie aby wstawic wszystkich usunietych klientow
		{
	
			to_adds = regret_cost_insertion(modified_solution,new_customers, avg_route_cost);
			//to_adds zawiera infomracje 
			//do jakiej trasy wstawic na jaka pozycje
			//oraz ktorego klienta z new_customers wstawic

		
	
			int insertion_route_index = to_adds.route_index;			 //wstaw do tej trasy
			int insertion_client_index = to_adds.client_index;			 //wstaw na ten indeks
			//Node selected_cusomter = new_customers[to_adds.client_index];// tego klienta TAK BYLO
			Node selected_cusomter = new_customers[to_adds.new_customer_index];// tego klienta TAK BYLO


			//wstawienie klienta i przeliczenie tras
		
			modified_solution[insertion_route_index].add_customer_at_index(selected_cusomter, insertion_client_index, 0.0, true);
			modified_solution[insertion_route_index].calculate_arrival_times();
			modified_solution[insertion_route_index].penatly_eta = to_adds.penalty; 	

			//usun wstawionego klienta z listy nie wstawionych
			//TUTAJ JEST BLAD
			
			int numer = to_adds.client_index;
			int rozmiar = new_customers.size();
			
			new_customers.erase(new_customers.begin() + to_adds.new_customer_index);
			
		}
		break;
	}

	return modified_solution;
}





//zwracamy lambda losowych klientow - ktorych bedziemy usuwac
std::vector<clientRatioInfo> random_removal(std::vector<Route>& solution, int lambda)
{
	clientRatioInfo move;
	std::vector<clientRatioInfo> to_remove;
	auto selected = get_n_random_clients(solution, lambda); // pobiranie lambda losowych klientow


	for (auto [route_idx, client_idx] : selected) 
	{
		int demand = solution[route_idx].customers[client_idx].demand;
		move = {route_idx, client_idx, double(demand) };
	}
	return to_remove;
}


//liczymy gain dla kazdego klienta, sortujemy malejaco a nasepnie wybieramy lambda klientow 
std::vector<clientRatioInfo> worst_removal(std::vector<Route>& solution,double avg_solution_cost, int lambda, double theta)
{
	
	
	double gain, ratio = 0.0;
	//n - ile jesz wszystkich klientow
	int demand,n, selected_index = 0;
	double penalty_eta = 0.0;
	std::vector<clientRatioInfo> ratio_i;
	std::vector<clientRatioInfo> result;
	


	for (int route = 0; route < solution.size(); ++route) // przejscie przez kazda trase
	{
		Route current_route = solution[route];
		int capacity = current_route.initial_capacity;
		int used_capacity = capacity - current_route.remaining_capacity;
		for (int client = 1; client < current_route.customers.size(); ++client) // client od 1 bo nie usuwam magazynu
		{
			gain = current_route.Z1[client] + calculate_penalty_hybrid(capacity, used_capacity - current_route.customers[client].demand, avg_solution_cost);
			ratio = current_route.customers[client].demand / gain;
			ratio_i.push_back({ route, client, ratio }); // czyli mam info o tym jaki to jest klient, oraz jaki ma wynik
		}
	}


	//sortowanie malejaco:
	std::sort(ratio_i.begin(), ratio_i.end(),
		[](const clientRatioInfo& a, const clientRatioInfo& b) {
			return a.ratio > b.ratio;
		});

	//wybor indeksu
	n = ratio_i.size();
	if (lambda > n) lambda = n;
	std::vector<bool> used(n, false);
	for (int i = 0; i < lambda; ++i) {
		int selected_index;
		do {
			double z = random_01();
			selected_index = static_cast<int>(ceil(pow(z, theta) * n)) - 1;  //-1 bo ceil zwraca od 1 do n, a indeks jest od 0 do n-1
			selected_index = std::clamp(selected_index, 0, n - 1);  
		} while (used[selected_index]);  

		used[selected_index] = true;
		result.push_back(ratio_i[selected_index]);
	}
	return result;

}

//obliczamy dystas pomiedzy kazdymi punktami w trasie d[i] d[i+1]
//sortujemy wszystkie dystnase rosnoco 
//wybor klintow analogicznie jak w worst_removal
//usuwamy dwoch klientow na raz 
std::vector<clientRatioInfo> worst_distance_removal(std::vector<Route>& solution, int lambda, double theta)
{
	std::vector<clientRatioInfo> distances;
	std::vector<clientRatioInfo> result;
	double distnace = 0.0;

	//1. obliczenie dystansow pomiedzy kazdymi sasiadami
	for (int route = 0; route < solution.size(); ++route) // przejscie przez kazda trase
	{
		Route current_route = solution[route];
		for (int i = 0; i < current_route.customers.size() - 1; ++i)
		{
			distnace = euclidean_distance(current_route.customers[i], current_route.customers[i + 1]);
			distances.push_back({ route, i, distnace }); // zapisanie infomracji od ktorego liczymy dystans
		}
	}

	if (distances.empty()) return {};

	//2. sortowanie rosnaco wszystkich odleglosci 
	std::sort(distances.begin(), distances.end(),
		[](const clientRatioInfo& a, const clientRatioInfo& b) {
			return a.ratio < b.ratio;
		});

	int n, selected_index = 0;

	n = distances.size();
	if (lambda > n) lambda = n;

	std::vector<bool> used(n, false);

	// klienci, uniklani, ktorzy zostali wybrani do usuniecia
	std::unordered_set<std::pair<int, int>, PairHash>   removed_clients;


	const int MAX_ATTEMPTS = lambda * 200;   // bezpieczna granica
	int attempts = 0;

	//wyszukiwanie klientow do usuniecia
	while (result.size() < lambda && attempts < MAX_ATTEMPTS) {
		attempts++;
		double z = random_01();                    // losowa liczba z [0,1)
		selected_index = static_cast<int>(ceil(pow(z, theta) * n)) - 1;  //-1 bo ceil zwraca od 1 do n, a indeks jest od 0 do n-1
		selected_index = std::clamp(selected_index, 0, n - 1);

		

		int random_selected_route_index1 = distances[selected_index].route_index;
		int random_selected_client_index1 = distances[selected_index].client_index;
		if (random_selected_client_index1 == 0)// nie usuwamy magazynu
		{
			continue;
		}

		int random_selected_route_index2 = distances[selected_index].route_index;
		int random_selected_client_index2 = distances[selected_index].client_index + 1;
		// jezli ktorykolwiek z klientow zostal wczesniej wybrany to pomijamy
		if (removed_clients.count({ random_selected_route_index1, random_selected_client_index1 }) || removed_clients.count({ random_selected_route_index2, random_selected_client_index2 }))
			continue;

		// Usuwamy obu klientów
		removed_clients.insert({ random_selected_route_index1, random_selected_client_index1 });
		removed_clients.insert({ random_selected_route_index2, random_selected_client_index2 });

		// Zapisujemy informacjê do wyniku (usuwamy obu)
		double distnace = distances[selected_index].ratio;
		result.push_back({ random_selected_route_index1, random_selected_client_index1, distnace });   // klient1
		result.push_back({ random_selected_route_index2, random_selected_client_index2, distnace });   // klient2
	}


	if (result.size() < lambda)
	{
		std::cout << "[WARNING] worst_distance_removal: udalo sie zebrac tylko: "
			<< result.size() << " z " << lambda << " klientow (po: "
			<< attempts << " probach)\n";
	}
	return result;
}

//Dzielimy cale rozwiaznie na sektory o zadnym kacie i nastepnie liczymy ile tras jest w danym sektorze
//czyli np od kata o do 15 stopni sa klienci kotrzy obslugiwani sa przez trasy A, B,B,C,A C to wynik to 3
std::vector<clientRatioInfo> confilcting_sector_removal(std::vector<Route>& solution, int lambda, double sector_size_deg)
{
	//dla kazdej trasy wyznaczyc sektor
	
	if (lambda <= 0)
	{
		return{};
	}


	//na ile czesci dzielmy trase
	const int num_sectors = static_cast<int>(360.0 / sector_size_deg);

	struct ClientPos {
		int route_idx;
		int client_idx;      // indeks w route.customers
		double angle_deg;
	};

	std::vector<ClientPos> all_clients;

	const Node& depot = {0,0.0,0.0,0};

	for (int r = 0; r < solution.size(); ++r) // przejscie przez wszystkie trasy
	{
		const Route& route = solution[r];
		for (int i = 1; i < route.customers.size(); ++i) // przejscie przez klientow
		{
			double angle = get_angle_deg(depot, route.customers[i]);
			all_clients.push_back({r,i, angle}); // zapisujemy klienta oraz jego kat
		}
	}

	if (all_clients.empty()) return {};
	if (all_clients.empty() || lambda > all_clients.size()) {
		lambda = all_clients.size();
	}

	//Losowanie punktu startowego - kata 0
	int start_client_idx = rand() % all_clients.size();
	double start_angle = all_clients[start_client_idx].angle_deg;
	//losowy kirunek
	int direction = (rand() % 2 == 0) ? 1 : -1;

	std::vector<ClientPos> clients_with_normalized;
	clients_with_normalized.reserve(all_clients.size());

	for (const auto& c : all_clients) {
	
		double diff = c.angle_deg - start_angle;

	
		double normalized = diff * direction;
		normalized = std::fmod(normalized, 360.0);
		if (normalized < 0) {
			normalized += 360.0;
		}

		clients_with_normalized.push_back({ c.route_idx, c.client_idx, normalized });
	}

	//przydzielenie klientow do sektorow
	std::vector<std::unordered_set<int>> sector_routes(num_sectors);

	for (const auto& c : clients_with_normalized) {
		int sector = static_cast<int>(c.angle_deg / sector_size_deg);
		sector_routes[sector].insert(c.route_idx);
	}

	//wyszukanie sektora z najwieksza iloscia roznych tras
	int best_sector = 0;
	size_t max_routes = 0;
	int count = 0; // licznik sektorow o takiej samej liczbie tras

	for (int s = 0; s < num_sectors; ++s) {
		size_t current_size = sector_routes[s].size();

		if (current_size > max_routes) {
			max_routes = current_size;
			best_sector = s;
			count = 1; // reset licznika dla nowej rekordowej wartosci
		}
		else if (current_size == max_routes && max_routes > 0) {
			count++;
			if ((std::rand() % count) == 0) {
				best_sector = s;
			}
		}
	}



	//Klienci z sketora w ktorym jest najwiecej roznych tras
	std::vector<ClientPos> sector_clients;
	for (const auto& c : clients_with_normalized) {
		int sector = static_cast<int>(c.angle_deg / sector_size_deg);
		if (sector == best_sector) {
			sector_clients.push_back(c);
		}
	}
	if (sector_clients.empty()) return {};

	//Losowe wybranie lambda klientow
	std::vector<clientRatioInfo> result;
	std::shuffle(sector_clients.begin(), sector_clients.end(),rng);

	int to_remove = std::min(lambda, static_cast<int>(sector_clients.size()));

	for (int i = 0; i < to_remove; ++i) {
		const auto& c = sector_clients[i];
		result.push_back({ c.route_idx, c.client_idx, 0.0});   
	}

	return result;

}


//zwraca infomracje na jaka pozcje wstawic new_customer
clientRatioInfo basic_greedy_insertion(std::vector<Route>& solution, Node& new_customer, double avg_cost)
{
	//wybieramy to przeniesienie ktore zwraca najmniejszy delta f
	double minimal_detla_f = std::numeric_limits<double>::max();
	double added_cost = 0.0;
	double delta_eta = 0.0;
	clientRatioInfo result{-1,-1,-1};

	for (int r = 0; r < solution.size(); ++r) // przjescie przez kazda trase
	{
		Route& route = solution[r];
		double route_capacity = route.initial_capacity - route.remaining_capacity;
		for (int i = 1; i <= route.customers.size(); i++)
		{
			if (route_capacity < new_customer.demand) // nie ma miejsca na klienta
			{
				continue;
			}
			delta_eta = calculate_penalty_hybrid(route.initial_capacity, route_capacity + new_customer.demand, avg_cost) - route.penatly_eta ;
			added_cost = route.calculate_cost_variation_of_inserting_customer(1, i, new_customer) + delta_eta;

			if (added_cost < minimal_detla_f)
			{
				minimal_detla_f = added_cost;
				result = {r,i,delta_eta };
			}
		}
	}
	return result;
	//jezeli nie znalziono miejsca to 3.2
	//wywolac perform_perturabtioan - patrz hybridavns - ale to juz poza ta funkjca
}


clientInfo regret_cost_insertion(std::vector<Route>& solution, std::vector<Node>& new_customers, double avg_cost)
{
	clientInfo result{0,0,0,0 };
	double max_regret = -1.0;

	for (int c = 0; c < new_customers.size(); ++c)
	{
		Node& customer = new_customers[c];
		double best_gain = -std::numeric_limits<double>::max();
		double best_delta_eta = 0.0;
		double second_best_gain = -std::numeric_limits<double>::max();

		int best_route_idx = -1;
		int best_pos = -1;

		std::vector<Node> wrapper = { customer };

		for (int r = 0; r < solution.size(); ++r)
		{
			Route& route = solution[r];
			double route_load = route.initial_capacity - route.remaining_capacity;

			if (route_load + customer.demand > route.initial_capacity * 2.0) continue;

			for (int i = 1; i <= (int)route.customers.size(); ++i)
			{
				double psi_plus = route.calculate_cost_variation_of_inserting_customer(1, i, customer);
		
				double delta_eta = route.penatly_eta - calculate_penalty_hybrid(route.initial_capacity, route_load + customer.demand, avg_cost);

	
				double current_gain = -psi_plus + delta_eta;

		
				if (current_gain > best_gain) {
					second_best_gain = best_gain;
					best_gain = current_gain;
					best_route_idx = r;
					best_pos = i;
					best_delta_eta = delta_eta;
				}
				else if (current_gain > second_best_gain) {
					second_best_gain = current_gain;
				}
			}
		}

		if (best_route_idx != -1) {
			
			double current_client_regret = best_gain - second_best_gain;

			//tylko jedna dostepna trasa 
			if (second_best_gain == -std::numeric_limits<double>::max()) {
				current_client_regret = std::numeric_limits<double>::max();
			}

			if (current_client_regret > max_regret)
			{
				max_regret = current_client_regret;
				result = { best_route_idx, best_pos, c,best_delta_eta };
			}
		}
	}

	


	return result;
}
