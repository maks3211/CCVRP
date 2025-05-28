#include "CvrpTypes.h"

//Zawiera vector klientow, id pojazdu, pozostala pojemnosc oraz koszt trasy
struct Route {
    std::vector<Node> customers;  // Lista ID klientów w trasie (w tym depozyt)
    int vehicle_id;  // Numer pojazdu
    int remaining_capacity;  // Pozosta³a pojemnoœæ pojazdu
    int initial_capacity;  // Pozosta³a pojemnoœæ pojazdu
    double route_cost;  // Ca³kowity koszt trasy (odleg³oœæ)
    double route_cost_improved; 
    Route(int vehicle_id, int capacity) : vehicle_id(vehicle_id), remaining_capacity(capacity), route_cost(0) {
        customers.push_back(Node(1,0,0,0));  // Dodaj depozyt na pocz¹tek trasy
		initial_capacity = capacity;  // Zapisz pocz¹tkow¹ pojemnoœæ
    }

    /// <summary>
	/// Adds a customer to the route if there is enough capacity left.
    /// </summary>
    /// <param name="customer"></param>
    /// <param name="distance"></param>
    /// <param name="force">if true adds costumer even wtih no space left</param>
    /// <returns></returns>
    bool add_customer(const Node& customer, const double distance,int position, bool force = false) {
        bool push = false;
       
        if (position >= customers.size() - 1)
        {
            push = true;
        }
        if (force || remaining_capacity >= customer.demand) {
            // Zaktualizuj trasê i pozosta³¹ pojemnoœæ
           // customers.push_back(Node(customer.id, customer.x, customer.y, customer.demand));
            if(push)
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


};