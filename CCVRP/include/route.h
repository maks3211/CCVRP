#pragma once
#include "cvrp_types.h"

#include <iostream>
//Zawiera vector klientow, id pojazdu, pozostala pojemnosc oraz koszt trasy
struct Route {
    std::vector<Node> customers;  // Lista ID klientów w trasie (w tym depozyt)
    int vehicle_id;  // Numer pojazdu
    int remaining_capacity;  // Pozostala pojemnosc pojazdu
    int initial_capacity;  // Pozosta³a pojemnosc pojazdu
    double route_cost;  // Calkowity koszt trasy (odleglosc)
    
    double penatly_eta = 0.0; // kara za przekroczenie pojemnosci (dla hybrid)
    bool is_penalized = false; // czy trasa jest penalizowana (dla hybrid)


    

	//Precompute dla hybrid 198 - sekcja 3.5.1
    std::vector<double> Z1; // dla hybrid - koszt usuniecia 1 klienta zaczynajac od pozucji i
    std::vector<double> Z2; // koszt usuniecia 2 kolejnych klientow zaczynajac od pozycji i

    //zrobic wpisywanie do tej tablicy podczas tworzenia rozwiazania/ obliczania kosztu skumulowanego aby odrazu ja uzpelniac 
    std::vector<double> arrival_times; // czas przyjazdu do danego klienta - czyli np [3] czas dotarca do klienta [3] - nie czas skumulowany poprstu czas przejazdu od 0->1->2->3
    double get_arrival_time(int pos) const { return (pos < arrival_times.size()) ? arrival_times[pos] : 0.0; }

    Route(int vehicle_id, int capacity);

    /// <summary>
    /// Adds a customer to the route if there is enough capacity left.
    /// </summary>
    /// <param name="customer"></param>
    /// <param name="distance"></param>
    /// <param name="force">if true adds costumer even wtih no space left</param>
    /// <returns></returns>
    bool add_customer(const Node& customer, const double distance, int position, bool force = false);

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
    bool add_customer_at_index(const Node& customer, int index, const double addded_cost, bool calculate_precompute = false);

    double calculate_removal_cost(int j, int u = 1);
    double calculate_insertion_cost(int insert_pos, const Node& u);

    double cumulative_cost_up_to(int pos);
    double cumulative_cost_after(int pos);
   
  
   

    bool remove_customer_at_index(int pos);
  

    void restore_customer_at(size_t pos, const Node& client);

    //przelicza dystans/ czas przyjazdu do kazdego klienta w trasie - zapisuje do arrival_times
    void recalculate_all();
    

    //// Dodane z mysla o metodzie hybrid - 198

	//uzupelnia/ aktualizuje arrival_times
	void calculate_arrival_times();
	//przelicza precompute dla trasy - czyli Z1 i Z2
    void update_precompute();
    /// <summary>
	/// Calculates the cost variation (psi) of removing a one or two customers starting from index i, according to equation 10 in section 3.5.1 of the hybrid method (198).
    /// </summary>
    /// <param name="u">number of customers to remove - 1 or 2 accroding to article</param>
    /// <param name="i">index of frist customer to remove</param>
    double calculate_cost_variation_of_removing_customer(int u, int i);

    /// <summary>
    /// 
    /// </summary>
    /// <param name="u">Number of inserting customers</param>
    /// <param name="i">Index of first inserting customer</param>
    /// <param name="new_customers"></param>
    /// <returns></returns>
    double calculate_cost_variation_of_inserting_customer(int u, int i, std::vector<Node>& new_customers);
    double calculate_cost_variation_of_inserting_customer(int u, int i, Node& new_customer);
	/// <summary>
	/// Calculates delta_G_plus for removing customers. Used in calculate_cost_variation_of_removing_customer function
	/// </summary>
	/// <param name="u">Number of clients to remove</param>
	/// <param name="i">Starting client index</param>
	/// <returns></returns>
	double calculate_delta_G_plus_for_removing_customers(int u, int i);


    double calculate_delta_G_plus_for_inserting_customers(int u, int j, std::vector<Node>& new_customers);

    ////Koniec dla hybrid


};


//zawiera cale rozwiazanie, czyli wszystkie trasy, calkowity koszt rozwiazania 
struct Result
{
    std::vector<Route> routes;
	double total_cost = 0.0;  
	double duration_seconds = 0.0;
};
