#pragma once
#include "cvrp_types.h"

#include <iostream>
//Zawiera vector klientow, id pojazdu, pozostala pojemnosc oraz koszt trasy
struct Route {
    std::vector<Node> customers;  // Lista ID klientów w trasie (w tym depozyt)
    int vehicle_id;  // Numer pojazdu
    int remaining_capacity;  // Pozosta³a pojemnoœæ pojazdu
    int initial_capacity;  // Pozosta³a pojemnoœæ pojazdu
    double route_cost;  // Ca³kowity koszt trasy (odleg³oœæ)
    double penatly_eta = 0.0; // kara za przekroczenie pojemnosci (dla hubrid)
    bool is_penalized = false; // czy trasa jest penalizowana (dla hybrid)
    Route(int vehicle_id, int capacity) : vehicle_id(vehicle_id), remaining_capacity(capacity), route_cost(0) {
        customers.push_back(Node(1, 0, 0, 0));  // Dodaj depozyt na pocz¹tek trasy
        initial_capacity = capacity;  // Zapisz pocz¹tkow¹ pojemnoœæ
    }

    /// <summary>
    /// Adds a customer to the route if there is enough capacity left.
    /// </summary>
    /// <param name="customer"></param>
    /// <param name="distance"></param>
    /// <param name="force">if true adds costumer even wtih no space left</param>
    /// <returns></returns>
    bool add_customer(const Node& customer, const double distance, int position, bool force = false) {
        bool push = false;

        if (position >= customers.size() - 1)
        {
            push = true;
        }
        std::cout << "COntorl";
        if (force || remaining_capacity >= customer.demand) {
            // Zaktualizuj trasê i pozosta³¹ pojemnoœæ
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
            // Dodaj koszt (odleg³oœæ)
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
    /// <returns></returns>
    /// 
    ///  ????????????czy robic nowego customera tak jak w add_customer czy skorzystac z przekazanego  ??
    bool add_customer_at_index(const Node& customer, int index, const double addded_cost)
    {
        //za duzy, nie dodatni

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
        return true;
    }


    //dodaje  -przeniesc to do utils bo jest problem z ta metoda bo sie zapletlaja pliki .h 
    bool add_customer_at_index_with_penalty(Node& client)
    {

        InsertionResult insert = find_best_insertion(*this, client);
        customers.insert(customers.begin() + insert.place, client);
        route_cost += insert.cost;
        remaining_capacity -= client.demand;
        is_penalized = true;
        //CZY MA BYC 50?? OD CZEGO TO ZALEZY
        penatly_eta += 50.0;
        return true;
    }



    bool remove_customer_at_index(int pos)
    {
        if (pos == 0 || pos >= customers.size()) {
            std::cerr << "B³¹d: remove_customer_at - nieprawid³owa pozycja " << pos << "\n";
            return false;
        }
        Node removed = customers[pos];
        remaining_capacity += removed.demand;
        customers.erase(customers.begin() + pos);
		return true;
    }

    void restore_customer_at(size_t pos, const Node& client)
    {
        if (pos > customers.size()) {
            pos = customers.size(); // zabezpieczenie - dodaj na koniec
        }

        // Wstawiamy klienta z powrotem
        customers.insert(customers.begin() + pos, client);

        // Aktualizacja danych trasy
        remaining_capacity -= client.demand;
    }

 
};


//zawiera cale rozwiazanie, czyli wszystkie trasy, calkowity koszt rozwiazania 
struct Result
{
    std::vector<Route> routes;
	double total_cost = 0.0;  
	double duration_seconds = 0.0;
};
