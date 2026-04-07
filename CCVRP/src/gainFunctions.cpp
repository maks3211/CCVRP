#include "gainFunctions.h"


double calculate_gain_1_insertion_hybrid(std::vector<Route>& solution, Move& move, double avg_cost, double beta, double gamma) {
    Route& r_from = solution[move.from_route];
    Route& r_to = solution[move.to_route];
    
	Node& client_to_move = r_from.customers[move.from_pos];
    //nowa pojemnosc trasy po wykonaniu przeniesienia 
    int new_used_capacity = r_from.initial_capacity - r_from.remaining_capacity - move.moved_capacity_from;
    double delta_eta_k = r_from.penatly_eta - calculate_penalty_hybrid(r_from.initial_capacity, new_used_capacity, avg_cost, beta, gamma);
   
    double delta_C_k = r_from.Z1[move.from_pos] - 0 + delta_eta_k;
    new_used_capacity = r_to.initial_capacity - r_to.remaining_capacity + move.moved_capacity_from;

    //warunek (5) za mocne przekorczenie kosztu
    //if (new_used_capacity > (1 + beta) * r_to.initial_capacity)
    //{
	//	return -std::numeric_limits<double>::infinity(); 
    //}
    double delta_eta_l = r_to.penatly_eta - calculate_penalty_hybrid(r_to.initial_capacity, new_used_capacity, avg_cost, beta, gamma);
    double delta_C_l = 0 - r_to.    calculate_cost_variation_of_inserting_customer(1,move.to_pos, client_to_move) + delta_eta_l;
	double gain = delta_C_k + delta_C_l;
    return gain;
}



double calculate_gain_1_1_exchange_hybrid(std::vector<Route>& solution, Move& move, double avg_cost, double beta, double gamma)
{
    Route& r_k = solution[move.from_route];
    Route& r_l = solution[move.to_route];

    Node& client_X = r_k.customers[move.from_pos];
    Node& client_Y = r_l.customers[move.to_pos];

    int demand_from = move.moved_capacity_from; // pojemnosc klienta przenosznego z trasy 'from' do trasy 'to'
	int demand_to = move.moved_capacity_to; // pojemnosc klienta przenoszonego z trasy 'to' do trasy 'from'

    //to continue - jest w gemini - szukac pod 2. Implementacja funkcji Gain dla 1-1 Exchange

   

    //trasa k (zabieramy X wstawiamy Y)
    int new_used_capacity_k = r_k.initial_capacity - r_k.remaining_capacity - move.moved_capacity_from + move.moved_capacity_to;

    //NIE WIEM CZY TO MA BYC CZY NIE, NIE MA ZABARDZO INFOMACJI O TYM
    //if (new_used_capacity_k > (1 + beta) * r_k.initial_capacity) //warunek 5  - 198
    //{
    //    return -std::numeric_limits<double>::infinity();
    //}

    //odrazu sprawdzam zbyt mocne naruszenie pojemnosci przed wykonywaniem obliczen
    int new_used_capacity_l = r_l.initial_capacity - r_l.remaining_capacity - move.moved_capacity_to + move.moved_capacity_from;
    //if (new_used_capacity_l > (1 + beta) * r_l.initial_capacity) //warunek 5  - 198
    //{
    //    return -std::numeric_limits<double>::infinity();
    //}

    double delata_eta_k = r_k.penatly_eta - calculate_penalty_hybrid(r_k.initial_capacity, new_used_capacity_k, avg_cost, beta, gamma);

    double psi_minus_k = r_k.Z1[move.from_pos];
    double psi_plus_k = r_k.calculate_cost_variation_of_inserting_customer(1,move.from_pos, client_Y);
	double delta_C_k = psi_minus_k - psi_plus_k + delata_eta_k;

	//trasa l (zabieramy Y wstawiamy X)
	
    double delata_eta_l = r_l.penatly_eta - calculate_penalty_hybrid(r_l.initial_capacity, new_used_capacity_l, avg_cost, beta, gamma);
    double psi_minus_l = r_l.Z1[move.to_pos];
    double psi_plus_l = r_l.calculate_cost_variation_of_inserting_customer(1, move.to_pos, client_X);
    double delta_C_l = psi_minus_l - psi_plus_l + delata_eta_l;

	double gain = delta_C_k + delta_C_l;
	return gain;

}




double calculate_gain_2_insertion_hybrid(std::vector<Route>& solution, Move& move, double avg_cost, double beta, double gamma)
{
     Route& r_from = solution[move.from_route];
     Route& r_to = solution[move.to_route];
    if (move.from_pos + 1 >= r_from.customers.size())
        return -1e9;   // blednych ruuch , brak sasiadow

    const Node& c1 = r_from.customers[move.from_pos];     // pierwszy klient pary
    const Node& c2 = r_from.customers[move.from_pos + 1]; // drugi klient pary

 

    double psi_plus_to = 0.0;
	std::vector<Node> new_customers = { c1, c2 };
    if (move.orientation == 0) // c1->c2
    {
        psi_plus_to = r_to.calculate_cost_variation_of_inserting_customer(2,move.to_pos, new_customers);
    }
    else
    {
		std::reverse(new_customers.begin(), new_customers.end()); // c2->c1
        psi_plus_to = r_to.calculate_cost_variation_of_inserting_customer(2, move.to_pos, new_customers);
    }

	double new_used_capacity_from = r_from.initial_capacity - r_from.remaining_capacity - c1.demand - c2.demand;
	double new_used_capacity_to = r_to.initial_capacity - r_to.remaining_capacity + c1.demand + c2.demand;


    double delata_eta_from = r_from.penatly_eta - calculate_penalty_hybrid(r_from.initial_capacity, new_used_capacity_from, avg_cost, beta, gamma);

    double psi_minus_from = r_from.Z2[move.from_pos];
   
    


    double delata_eta_to = r_to.penatly_eta - calculate_penalty_hybrid(r_to.initial_capacity, new_used_capacity_to, avg_cost, beta, gamma);


	double delta_C_from = psi_minus_from - 0 + delata_eta_from; // usuwamy 2 klientow wiec psi plus to 0
	double delta_C_to = 0 - psi_plus_to + delata_eta_to; // wstawiamy 2 klientow wiec psi minus to 0
	double gain = delta_C_from + delta_C_to;
    return gain;
}

//w obrebie jednej trasy 
double calculate_gain_2_opt_hybrid(std::vector<Route>& solution, Move& move, double avg_cost) {
    Route& r = solution[move.from_route];
    int i = move.from_pos;              // pocz¹tek odwracania
    int j = i + move.number_of_moved_clients - 1; // koniec odwracania (indeks ostatniego elementu w bloku)

    double psi_minus = r.calculate_cost_variation_of_removing_customer(move.number_of_moved_clients, i);

    std::vector<Node> segment(r.customers.begin() + i, r.customers.begin() + j + 1);
    std::reverse(segment.begin(), segment.end());


    double psi_plus = r.calculate_cost_variation_of_inserting_customer(move.number_of_moved_clients, i, segment);

    return psi_minus - psi_plus;        //pojemnosc sie nie zmienia

}
double calculate_gain_2_opt_prim_hybrid(std::vector<Route>& solution, Move& move, double avg_cost, double beta = 0.05, double gamma = 0.6)
{
    Route& k_route = solution[move.from_route];
    Route& l_route = solution[move.to_route];

    //Wyznaczenie delta_C_k - czayli gain pierwszej trasy

    //zmiana kosztu po usunieciu klientow z trasy k

    int tail_k_length = move.number_of_moved_clients;
    int tail_l_length = move.number_of_moved_clients_b;

    //klienci z trasy k ktorzy trafia to trasy l
    std::vector<Node> customers_from_k(k_route.customers.begin() + move.from_pos, k_route.customers.end());

    std::vector<Node> customers_from_l(l_route.customers.begin() + move.to_pos, l_route.customers.end());
   

    double psi_plus_k = k_route.calculate_cost_variation_of_inserting_customer(tail_l_length, move.from_pos, customers_from_l);
    double psi_minus_k = k_route.calculate_cost_variation_of_removing_customer(tail_k_length, move.from_pos);


    double psi_plus_l = l_route.calculate_cost_variation_of_inserting_customer(tail_k_length, move.to_pos, customers_from_k);
    double psi_minus_l = l_route.calculate_cost_variation_of_removing_customer(tail_l_length, move.to_pos);

    double new_k_demand = k_route.initial_capacity - k_route.remaining_capacity - move.moved_capacity_from + move.moved_capacity_to;
    double new_l_demand = l_route.initial_capacity - l_route.remaining_capacity - move.moved_capacity_to + move.moved_capacity_from;

    double delta_eta_k = k_route.penatly_eta - calculate_penalty_hybrid(k_route.initial_capacity, new_k_demand, avg_cost, beta, gamma);
    double delta_eta_l = l_route.penatly_eta - calculate_penalty_hybrid(l_route.initial_capacity,new_l_demand, avg_cost, beta, gamma);



    double delta_C_k = psi_minus_k - psi_plus_k + delta_eta_k;
    double delta_C_l = psi_minus_l - psi_plus_l + delta_eta_l;
    double gain = delta_C_k + delta_C_l;

    return gain; 
}


std::vector<Move> calculate_gain_cross_tail_hybrid(std::vector<Route>& solution, Move& move, double avg_cost, double beta = 0.05, double gamma = 0.6)
{
    Route& k_route = solution[move.from_route];
    Route& l_route = solution[move.to_route];

    int tail_k_length = move.number_of_moved_clients;
    int tail_l_length = move.number_of_moved_clients_b;


    //klienci kotrzy sa przenoszeni - nie zmieniona kolejnosc 
    std::vector<Node> customers_from_k(k_route.customers.begin() + move.from_pos, k_route.customers.end());
    std::vector<Node> customers_from_l(l_route.customers.begin() + move.to_pos, l_route.customers.end());


    //kara nie zalezna od tego czy odwroceni klienci
    int new_k_demand = k_route.initial_capacity - k_route.remaining_capacity - move.moved_capacity_from + move.moved_capacity_to;
    int new_l_demand = l_route.initial_capacity - l_route.remaining_capacity - move.moved_capacity_to + move.moved_capacity_from;

    double delta_eta_k = k_route.penatly_eta - calculate_penalty_hybrid(k_route.initial_capacity, new_k_demand, avg_cost, beta, gamma);
    double delta_eta_l = l_route.penatly_eta - calculate_penalty_hybrid(l_route.initial_capacity, new_l_demand, avg_cost, beta, gamma);

    double gain = 0.0;

    std::vector<Move> moves;

    //nowe - koszt wstawienia zalezny od kolejnosci
    double psi_plus_k = k_route.calculate_cost_variation_of_inserting_customer(tail_l_length, move.from_pos, customers_from_l);
    double psi_plus_l = l_route.calculate_cost_variation_of_inserting_customer(tail_k_length, move.to_pos, customers_from_k);

    std::reverse(customers_from_k.begin(), customers_from_k.end());
    std::reverse(customers_from_l.begin(), customers_from_l.end());

    // psi w ktorym dodany fragment jest odwrocony
    double psi_plus_k_reversed = k_route.calculate_cost_variation_of_inserting_customer(tail_l_length, move.from_pos, customers_from_l);
    double psi_plus_l_reversed = l_route.calculate_cost_variation_of_inserting_customer(tail_k_length, move.to_pos, customers_from_k);

    //koszt usuniecia jest niezlezny od kolejnosci
    double psi_minus_k = k_route.calculate_cost_variation_of_removing_customer(tail_k_length, move.from_pos);
    double psi_minus_l = l_route.calculate_cost_variation_of_removing_customer(tail_l_length, move.to_pos);

    for (int orientation = 4; orientation >= 1; --orientation)
    {
        if (orientation == 4) // bez odwracania
        {
            gain = (psi_minus_k - psi_plus_k + delta_eta_k) + (psi_minus_l - psi_plus_l + delta_eta_l);
        }
       

        //trasa k ma klientow z trasy l bez zmienionej kolejnosci
        // trasa l ma klientow z trasy k z odwrocona kolejnoscia
        if (orientation == 3) // odwracamy tylko framgnet k- ten ktory trafia do trasy l- czyli odwracamy customers_from_k 
        {
            gain = (psi_minus_k - psi_plus_k + delta_eta_k) + (psi_minus_l - psi_plus_l_reversed + delta_eta_l);
        }

        //trasa k ma klientow z trasy l z odwrocona kolejnoscia fragmentu l
        //trasa l ma klientow z trasy k w oryginalej kolejnosci fragmentu k
        else if(orientation == 2) // odwracamy tylko fragment l 
        {
            gain = (psi_minus_k - psi_plus_k_reversed + delta_eta_k) + (psi_minus_l - psi_plus_l + delta_eta_l);
        }

        else if (orientation == 1) //odwracamy obie trasy
        {        
            gain = (psi_minus_k - psi_plus_k_reversed + delta_eta_k) + (psi_minus_l - psi_plus_l_reversed + delta_eta_l);
        }
      
        Move my_move = move;
        my_move.orientation = orientation;
        my_move.gain = gain;
        moves.push_back(my_move);
    }
 
	return moves; 
}