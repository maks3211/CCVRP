#pragma once
#include "route.h"
#include "utils.h"

//Zawiera vector klientow, id pojazdu, pozostala pojemnosc oraz koszt trasy


    Route::Route(int vehicle_id, int capacity) : vehicle_id(vehicle_id), remaining_capacity(capacity), route_cost(0) {
        customers.push_back(Node(1, 0, 0, 0));  // Dodaj depozyt na początek trasy
        initial_capacity = capacity;  // Zapisz początkową pojemnosc
    }

    /// <summary>
    /// Adds a customer to the route if there is enough capacity left.
    /// </summary>
    /// <param name="customer"></param>
    /// <param name="distance"></param>
    /// <param name="force">if true adds costumer even wtih no space left</param>
    /// <returns></returns>
    bool Route::add_customer(const Node& customer, const double distance, int position, bool force) {
        bool push = false;

        if (position >= customers.size() - 1)
        {
            push = true;
        }
        if (force || remaining_capacity >= customer.demand) {
            // Zaktualizuj trasę i pozostałą pojemność
           // customers.push_back(Node(customer.id, customer.x, customer.y, customer.demand));
            if (push)
            {
                customers.push_back(Node(customer.id, customer.x, customer.y, customer.demand));
            }
            else
            {
                customers.insert(customers.begin() + (position + 1), Node(customer.id, customer.x, customer.y, customer.demand));
            }
            remaining_capacity -= customer.demand;
            // Dodaj koszt (odległość)
            route_cost += distance;
            return true;
        }
        return false;
    }

    /// <summary>
    /// Adds a customer to the route at a specific index 
    /// </summary>
    /// <param name="customer"></param>
    /// <param name="index">Index at which new customer will be placed - to add at the end pass customer.size</param>
    /// <param name="addded_cost"></param>
    /// <param name="calculate_precompute">Added becaused of hybrid method</param>

    /// <returns></returns>
    /// 
    ///  ????????????czy robic nowego customera tak jak w add_customer czy skorzystac z przekazanego  ??
    bool Route::add_customer_at_index(const Node& customer, int index, const double addded_cost, bool calculate_precompute)
    {
        //za duzy lub nie dodatni
        if (index > customers.size() || index <= 0)
        {
            return false;
        }
        //wstawianie na koniec
        if (index == customers.size())
        {
            customers.push_back(customer);
        }
        else
        {
            customers.insert(customers.begin() + index, customer);
        }
        route_cost += addded_cost;
        remaining_capacity -= customer.demand;

        if (calculate_precompute)
        {
            update_precompute();
        }

        return true;
    }


    /// <summary>
	/// Calculates psi - (u) for k route - equation 10 in hybrid - 198 - sekcja 3.5.1
    /// </summary>
    /// <param name="j"> indeks pierwszego usnietego klienta</param>
    /// <param name="u">liczba usuwanych klientow 1 lub 2</param>
    /// <returns></returns>
    double Route::calculate_removal_cost(int j, int u) {
        // j - indeks pierwszego usuwanego klienta
        // u - liczba usuwanych klientów (1 dla Z1, 2 dla Z2)
        int n_k = (int)customers.size() - 1; // liczba klientów (bez depot na pozycji 0)
        int i = j - 1; // klient przed usuwanym blokiem
        int p = j + u; // klient po usuwanym bloku

        // 1. Suma czasów przyjazdu usuwanych klientów (pierwszy człon równania 10)
        double sum_t = 0.0;
        for (int k = 0; k < u; ++k) {
            sum_t += arrival_times[j + k];
        }

        // 2. Obliczanie Delta G+ (równanie w sekcji 3.5.1)
        double delta_G_plus = 0.0;

        // Skoro NIE WRACAMY do bazy:
        // Jeżeli usuwamy blok na samym końcu trasy, to 'p' wskazywałoby poza wektor.
        // Wtedy zmiana dystansu (Delta G+) nie wpływa na nikogo dalej, bo trasa się tam kończy.
        if (p < (int)customers.size()) {
            double d_i_p = euclidean_distance(customers[i], customers[p]);
            double path_distance_i_to_p = arrival_times[p] - arrival_times[i];

            delta_G_plus = d_i_p - path_distance_i_to_p;
        }

        // 3. Liczba klientów pozostających w trasie ZA blokiem
        int customers_after_block = (p <= n_k) ? (n_k - p + 1) : 0;

        // Równanie 10: Psi = Suma_t + (liczba_pozostałych * Delta_G_plus)
        return sum_t + (customers_after_block * delta_G_plus);
    }

    double Route::calculate_insertion_cost(int insert_pos, const Node& u)
    {
        int n = (int)customers.size();

        // 1. Czas przyjazdu do nowego klienta u
        // Czas dotarcia do poprzednika + dystans do u
        double t_u = arrival_times[insert_pos - 1] + euclidean_distance(customers[insert_pos - 1], u);

        // 2. Obliczamy Delta G+ (o ile wydłuży się droga dla kolejnych klientów)
        double delta_G_plus = 0.0;

        // Jeśli wstawiamy w środek trasy (istnieje klient na pozycji insert_pos)
        if (insert_pos < n) {
            double d_prev_u = euclidean_distance(customers[insert_pos - 1], u);
            double d_u_next = euclidean_distance(u, customers[insert_pos]);
            double d_prev_next = euclidean_distance(customers[insert_pos - 1], customers[insert_pos]);

            delta_G_plus = d_prev_u + d_u_next - d_prev_next;
        }
        // Jeśli insert_pos == n (wstawianie na sam koniec), delta_G_plus wynosi 0.0
        // ponieważ nikt nie jedzie dalej i nie ma krawędzi powrotnej do bazy.

        // 3. Wzór 11: t_u + (liczba klientów za nim * delta_G)
        int remaining_after = n - insert_pos;

        return t_u + (remaining_after * delta_G_plus);
    }



    // liczy koszt dotarcia od magazynu do klienta na indeksie pos 
    double Route::cumulative_cost_up_to(int pos)
    {
        if (pos < 0) return 0.0;
        if (pos >= customers.size()) pos = customers.size() - 1;

        double sum = 0.0;
        double current_time = 0.0;

        for (int i = 1; i <= pos; ++i)       // zaczynamy od 1, bo 0 = depot
        {
            current_time += euclidean_distance(customers[i - 1], customers[i]);
            sum += current_time;
        }
        return sum;
    }


	//liczy koszt dotarcia od klienta na indeksie pos do konca trasy
    double Route::cumulative_cost_after(int pos)
    {
        if (pos >= customers.size() - 1) return 0.0;

		double sum = 0.0;
        double current_time = 0.0;
        for (int i = pos + 1; i < customers.size(); ++i)
        {
            current_time += euclidean_distance(customers[i - 1], customers[i]);
            sum += current_time;
        }
		return sum;
    }



    bool Route::remove_customer_at_index(int pos)
    {
        if (pos == 0 || pos >= customers.size()) {
            std::cerr << "Błąd: remove_customer_at - nieprawidlowa pozycja " << pos << "\n";
            return false;
        }
        Node removed = customers[pos];
        remaining_capacity += removed.demand;
        customers.erase(customers.begin() + pos);
		return true;
    }

    void Route::restore_customer_at(size_t pos, const Node& client)
    {
        if (pos > customers.size()) {
            pos = customers.size(); // zabezpieczenie - dodaj na koniec
        }

        // Wstawiamy klienta z powrotem
        customers.insert(customers.begin() + pos, client);

        // Aktualizacja danych trasy
        remaining_capacity -= client.demand;
    }


    void Route::calculate_arrival_times()
    {
        arrival_times.clear();
        arrival_times.push_back(0.0); // Depot
        cumulative_costs.clear();
        cumulative_costs.push_back(0.0);
        double current_time = 0.0;
        double total_cumulative = 0.0;

        for (size_t i = 1; i < customers.size(); ++i) {
            current_time += euclidean_distance(customers[i - 1], customers[i]);
            arrival_times.push_back(current_time);            
            total_cumulative += current_time;
            cumulative_costs.push_back(total_cumulative);
        }
        route_cost = total_cumulative;
    }


   
    void Route::update_precompute()
    {
        int n = customers.size();
        Z1.assign(n, 0.0);
        Z2.assign(n, 0.0);
        for (int i = 1; i < n; ++i)
        {
            // Z1[i] = ψ⁻ dla u=1
            Z1[i] = calculate_cost_variation_of_removing_customer(1, i);
            // Z2[i] = ψ⁻ dla u=2 (jeśli jest drugi klient)
            if (i + 1 < n)
                Z2[i] = calculate_cost_variation_of_removing_customer(2, i);
        }
    }


    // u - ile klientow usuwam
    //i = od ktorego klienta usuwam'
    //PSI- 
    double Route::calculate_cost_variation_of_removing_customer(int u, int i)
    {

        if (i < 1 || i + u - 1 >= customers.size())
            return 0.0;

        double distnace_sum = 0.0;
      
        for (int p = i; p <= i + u - 1; p++)
        {
			distnace_sum += arrival_times[p];
        }

        int n_k = customers.size() - 1;
      
        int remaining = n_k - (i + u - 1);

        double psi_minus = distnace_sum + remaining * calculate_delta_G_plus_for_removing_customers(u,i);

        return psi_minus;   


    }


    double Route::calculate_delta_G_plus_for_removing_customers(int u, int i)
    {
        if (i + u > customers.size())
            return 0.0;

        double delta_G = 0.0;
        for (int a = i-1; a <= i+u-1; a++)
        {
			delta_G += euclidean_distance(customers[a], customers[a + 1]);
        }
        double new_edge = (i + u < customers.size())
            ? euclidean_distance(customers[i - 1], customers[i + u])
            : 0.0;
	
        return delta_G - new_edge;
    }





    double Route::calculate_cost_variation_of_inserting_customer(int u, int j, Node& new_customer)
    {
        std::vector<Node> temp = { new_customer };
        return calculate_cost_variation_of_inserting_customer(u, j, temp);
    }

    //u ile klientow wstawiam
    //j od ktorego klienta wstawiam == pierwszy niowy klient trafia na j index
	//customers - klienci ktorych wstawiam - musze znac ich dane, zeby obliczyc dystanse
    // //PSI+
    double Route::calculate_cost_variation_of_inserting_customer(int u, int j, std::vector<Node>& new_customers)
    {
        //Suma czasow dojazdu do wstawionych klientow
        //t prim od l
       
        if (j < 1 || j > customers.size() || new_customers.size() != u)
            return 0.0;
      
        double current_arrival_time  = arrival_times[j - 1];
        double sum_t_prime = 0.0;
        for (int p = 0; p < new_customers.size(); p++)
        {
            double d = (p == 0) ? euclidean_distance(customers[j - 1], new_customers[p])
                : euclidean_distance(new_customers[p - 1], new_customers[p]);
            current_arrival_time += d;      // To jest czas przyjazdu do p-tego wstawianego klienta
            sum_t_prime += current_arrival_time; // Dodajemy go do sumy (to jest to t' z PDF)
        }

        int remaining = (customers.size() - 1 ) - j + 1;

        double psi_plus = sum_t_prime - remaining * calculate_delta_G_plus_for_inserting_customers(u, j, new_customers);

        return psi_plus;

    }



    //j - startowa pozycja w nowej trasie
    //u - liczba wstawianych klientow
	//l - trasa do ktorej wstawiam
    // 
    //k - trasa z ktorej usuwam
    //i - startowa pozycja usunietego elmentu z trasy k
    
    double Route::calculate_delta_G_plus_for_inserting_customers(int u, int j, std::vector<Node>& new_customers)
    {

        if (j < 1 || j > customers.size())
            return 0.0;
       
		double delta_G_plus = 0.0;

        //d Rl j-1, Rl j 
        double first_part = 0.0;



        //d Rl j-1, Ri k 
		double second_part = euclidean_distance(customers[j - 1], new_customers[0]);

        //suma 
		double added_customers_distance = 0.0;

        for (int a = 0; a <= u - 2; a++)
        {
            added_customers_distance += euclidean_distance(new_customers[a], new_customers[a + 1]);
        }
        //d Rk i+u-1, Rl j
		double last_new_customer_to_next_old = 0.0;

        if (j < customers.size()) {
            //d Rk i+u-1, Rl j
            last_new_customer_to_next_old = euclidean_distance(new_customers.back(), customers[j]);

            //d Rl j-1, Rl j 
            first_part = euclidean_distance(customers[j - 1], customers[j]);
        }
        delta_G_plus = first_part - (second_part + added_customers_distance + last_new_customer_to_next_old);

		return delta_G_plus;
    }
    //AND THENIMPLENT GAIN FUNCTIONS IN GAINFUNCTIONS.CPP FILE - so psi minus and psi plus will be used, and after that implent delta penalty function



 

    void Route::recalculate_all()
    {
        calculate_arrival_times();
        update_precompute(); 

    }


