#include "CvrpTypes.h"

//Zawiera vector klientow, id pojazdu, pozostala pojemnosc oraz koszt trasy
struct Route {
    std::vector<int> customers;  // Lista ID klientów w trasie (w tym depozyt)
    int vehicle_id;  // Numer pojazdu
    int remaining_capacity;  // Pozosta³a pojemnoœæ pojazdu
    double route_cost;  // Ca³kowity koszt trasy (odleg³oœæ)

    Route(int vehicle_id, int capacity) : vehicle_id(vehicle_id), remaining_capacity(capacity), route_cost(0) {
        customers.push_back(1);  // Dodaj depozyt na pocz¹tek trasy
    }

    /// <summary>
	/// Adds a customer to the route if there is enough capacity left.
    /// </summary>
    /// <param name="customer"></param>
    /// <param name="distance"></param>
    /// <param name="force">if true adds costumer even wtih no space left</param>
    /// <returns></returns>
    bool add_customer(const Node& customer, const double distance, bool force = false) {
        std::cout << "Adding new customer";
        if (!force && remaining_capacity >= customer.demand) {
            // Zaktualizuj trasê i pozosta³¹ pojemnoœæ
            customers.push_back(customer.id);
            remaining_capacity -= customer.demand;
            // Dodaj koszt (odleg³oœæ)
            route_cost += distance;
            return true;
        }
        else if(force)
        {
            customers.push_back(customer.id);
            remaining_capacity -= customer.demand;
            route_cost += distance;
            return true;
        }
        return false;  
    }
};