#include "single_route_improvement.h"


void single_route_improvement(Route& route, int iterations)
{
	int three_opt_times = 0;
	double old_old_cost = route.route_cost;
	int N = route.customers.size() - 1;
	if (N < 6) return; // trasa jest za krotka
	//std::vector<int> points;

	for (int i = 0; i < iterations; ++i)
	{
		Route changed_route = route;
		//points.clear();
		int choice = random_int_from_to(0, 1);
		
		if (N < 9) choice = 0; // trasa za krotka na 4-opt - wykonaj 3-opt
		if (choice == 0) // 3-opt
		{
			three_opt_times++;
		
			//pierwotne rozwiazanie - calkowice losowe przedzialy
			//while (points.size() < 3) { // losowanie przedzialow
			//	int p = random_int_from_to(0, N-2); 
			//	if (std::find(points.begin(), points.end(), p) == points.end())  // jezeli nie ma jeszce tago przedzialu
			//	{
			//		points.push_back(p);
			//	}
			//}
			//std::sort(points.begin(), points.end()); // trzy konce przedzialow 
		//	int i = 1 + rand() % (N - 5);
		//	int j = i + 2 + rand() % (N - i - 4);
		//	int k = j + 2 + rand() % (N - j - 2);


			int i = random_int_from_to(1, N - 5);
			int j = random_int_from_to(i + 2, N - 3);
			int k = random_int_from_to(j + 2, N - 1);
		
			perform_3_opt(changed_route, i, j, k);  // w changed route_jest juz od razu zmieniena trasa

		}
		else // 4-opt
		{
			//while (points.size() < 4) {
			//	int p = random_int_from_to(0, N - 2);
			//	if (std::find(points.begin(), points.end(), p) == points.end())  // jezeli nie ma jeszce tago przedzialu
			//	{
			//		points.push_back(p);
			//	}
			//}
			//std::sort(points.begin(), points.end()); // trzy konce przedzialow 
		//	int i = 1 + rand() % (N - 7);
		//	int j = i + 2 + rand() % (N - i - 6);
		//	int k = j + 2 + rand() % (N - j - 4);
		//	int l = k + 2 + rand() % (N - k - 2);
			
			int i = random_int_from_to(1, N - 7);
			int j = random_int_from_to(i + 2, N - 5);
			int k = random_int_from_to(j + 2, N - 3);
			int l = random_int_from_to(k + 2, N - 1);

			perform_4_opt(changed_route, i, j, k, l);
		}
		std::vector<Route> tmp = { changed_route };
		//std::cout << "\n						 KOSZT TRASY wejscia    : " << old_old_cost;
		//std::cout << "\n						 KOSZT TRASY PRZED 2-OPT: " << tmp[0].route_cost;
		

		while (perform_first_improvement_2_opt(tmp))
		{
			//wykonuj 2-opt dopoki poprawa
			
		}
		//std::cout << "\n						 KOSZT TRASY po 2-OPT: " << tmp[0].route_cost << "\n";
		changed_route = tmp[0];
		
		if (std::abs(tmp[0].route_cost - route.route_cost) < 0.01) // jest poprawa z drobnym marginesem
		{
			
			route = tmp[0];
		}
	
	//	std::cout << "\n=-=-=-=-=-=-=-=-=-=-=-=-STARY  KOSZT:" << old_old_cost << "  Nowy koszt: " << tmp[0].route_cost;
	}
	
	//std::cout << "\noperator 3-opt zostal wykonany: " << three_opt_times << " co daje: " << (float)three_opt_times / iterations * 100 << "%";
	
}


void perform_3_opt(Route& route,int i, int j, int k)
{
	std::vector<Node> S1(route.customers.begin()        , route.customers.begin() + i + 1);
	std::vector<Node> S2(route.customers.begin() + i + 1, route.customers.begin() + j + 1);
	std::vector<Node> S3(route.customers.begin() + j + 1, route.customers.begin() + k + 1);
	std::vector<Node> S4(route.customers.begin() + k + 1, route.customers.end());
	int variant = random_int_from_to(1, 4);
	route.customers.clear();
	route.customers.insert(route.customers.end(), S1.begin(), S1.end()); // wstawiamy na koniec S1
	
	switch (variant) {
		case 1:		//S1-S3-S2-S4
			route.customers.insert(route.customers.end(), S3.begin(), S3.end());  //jest teraz S1-S3
			route.customers.insert(route.customers.end(), S2.begin(), S2.end());  //jest teraz S1-S3-S2
			break;
		case 2: //S1-S3(rev)-S2-S4
			std::reverse(S3.begin(), S3.end());
			route.customers.insert(route.customers.end(), S3.begin(), S3.end());
			route.customers.insert(route.customers.end(), S2.begin(), S2.end());
			break;
		case 3: //S1-S3-S2(rev)-S4
			std::reverse(S2.begin(), S2.end());
			route.customers.insert(route.customers.end(), S3.begin(), S3.end());
			route.customers.insert(route.customers.end(), S2.begin(), S2.end());
			break;
		case 4: //S1-S2(rev)-S3(rev)-S4
			std::reverse(S2.begin(), S2.end());
			std::reverse(S3.begin(), S3.end());
			route.customers.insert(route.customers.end(), S3.begin(), S3.end());
			route.customers.insert(route.customers.end(), S2.begin(), S2.end());
			break;
	}
	route.customers.insert(route.customers.end(), S4.begin(), S4.end());
	route.calculate_arrival_times(); // przeliczenie kosztu trasy
}

//WERSJA double bridge - S1 S4 S3 S2 S5
void perform_4_opt(Route& route, int i, int j, int k, int l)
{
	std::vector<Node> S1(route.customers.begin(), route.customers.begin() + i + 1);
	std::vector<Node> S2(route.customers.begin() + i + 1, route.customers.begin() + j + 1);
	std::vector<Node> S3(route.customers.begin() + j + 1, route.customers.begin() + k + 1);
	std::vector<Node> S4(route.customers.begin() + k + 1, route.customers.begin() + l + 1);
	std::vector<Node> S5(route.customers.begin() + l + 1, route.customers.end());
	route.customers.clear();
	route.customers.insert(route.customers.end(), S1.begin(), S1.end());
	route.customers.insert(route.customers.end(), S4.begin(), S4.end());
	route.customers.insert(route.customers.end(), S3.begin(), S3.end());
	route.customers.insert(route.customers.end(), S2.begin(), S2.end());
	route.customers.insert(route.customers.end(), S5.begin(), S5.end());
	route.calculate_arrival_times();

}