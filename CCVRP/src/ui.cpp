#include "Ui.h"




Ui::Ui(brainConfig& bso_config, hybridAvnsLnsConfig& hybrid_config, SkewedVNSConfig& sekwed_config,
	int& number_bso, int& number_hybrid, int& number_skewed,
	std::string& input_path, std::string& instance_name, int& num_vehicles,
	std::string& result_path, std::string& result_folder,
	bool settings_loaded)
	: bso_config(bso_config), hybrid_config(hybrid_config), sekwed_config(sekwed_config),
	number_bso(number_bso), number_hybrid(number_hybrid), number_skewed(number_skewed),
	input_path(input_path), instance_name(instance_name), num_vehicles(num_vehicles),
	result_path(result_path), result_folder(result_folder),
	settings_loaded(settings_loaded)

{

}
void Ui::main_menu()
{
	bool run = true;
	while (run)
	{
		system("cls");
		if (!settings_loaded)
		{
			std::cout << "\nNie znaleziono pliku 'settings.json' \nWczytano konfiguracje domyslna" << std::endl;
		}
		
		std::cout << "\n                ======= Cumulative Capacitated Vehicle Routing Problem  (CCVRP) =======\n";
		std::cout << "\n1. Edycja parametrow";
		std::cout << "\n2. Liczba uruchomien";
		std::cout << "\n3. Wejscie problemu";
		std::cout << "\n4. Sciezka zapisu";
		std::cout << "\n0. START";

		std::cout << "\nWybierz opcje: ";
		int choice;
		std::cin >> choice;
		std::cin.ignore(10000, '\n');
		switch (choice)
		{
		case 0:
		{
			system("cls");
			run = false;
			break;
		}
		case 1:
		{
			system("cls");
			set_params();
			
			break;
		}
		case 2:
		{

			system("cls");
			set_number_of_runs();
			
			break;
		}
		case 3:
		{
			system("cls");
			set_input_path();		
			break;
		}
		case 4:
		{
			system("cls");
			set_result_path();
			break;
		}
		default:
		{
			std::cout << "\nNieporawna wartosc";
		}

		}
	}
}

void Ui::set_params()
{
	bool run = true;
	while (run)
	{
		system("cls");
		std::cout << "\n======= MODYFIKACJA PARAMETOR ALGORYTMOW =======\n";
		std::cout << "\n1. BSO";
		std::cout << "\n2. HYBRID";
		std::cout << "\n3. SKEWED";
		std::cout << "\n0. Wyjscie";

		std::cout << "\nWybierz opcje: ";
		int choice;
		std::cin >> choice;
		std::cin.ignore(10000, '\n');
		switch (choice)
		{
		case 0:
		{
			run = false;
			break;
		}
		case 1:
		{
			system("cls");
			set_bso_params(bso_config);
			
			break;
		}
		case 2:
		{
			set_hybrid_params(hybrid_config);
			system("cls");
			break;
		}
		case 3:
		{
			set_skewed_params(sekwed_config);
			system("cls");
			break;
		}
		default:
		{
			std::cout << "\nNieporawna wartosc";
		}

		}
	}
}

void Ui::set_bso_params(brainConfig& config)
{
	bool run = true;
	while (run)
	{
		system("cls");
		std::cout << "\n======= Aktualne wartosci konfiguracyjne Brain Storm Optimalization =======\n";
		std::cout << "\n1. T1 (50) = " << config.T1   << "		Single route improvement iteration number"	;
		std::cout << "\n2. T2 (5) = " << config.T2 << "		Divergent operation iteration number";
		std::cout << "\n3. alfa_1 (0.05) = " << config.alfa_1 << "		 Perturbation on the best-so-far solution elemnts count multiplayer";
		std::cout << "\n4. alfa_2 (0.05) = " << config.alfa_2 << "		 Divergent operation elemnts count multiplayer";
		std::cout << "\n5. N (150) = " << config.N << "		Number of new solutions generating for each subproblem";
		std::cout << "\n6. Main loop itarations (4) = " << config.main_loop_itarations;
		std::cout << "\n7. Single route improvement margin (0.01) = " << config.single_route_improvement_margin;
		std::cout << "\n0. Wyjscie\n";

		std::cout << "Wybierz opcje: ";
		int choice;
		std::cin >> choice;
		std::cin.ignore(10000, '\n');
		switch (choice)
		{
			case 0:
			{
				run = false;
				break;
			}
			case 1:
			{
				print_type_in_value_for("T1");
				std::cin >> config.T1;
				system("cls");
				break;
			}
			case 2:
			{
				print_type_in_value_for("T2");
				std::cin >> config.T2;
				system("cls");
				break;
			}
			case 3:
			{
				print_type_in_value_for("alfa_1");
				std::cin >> config.alfa_1;
				system("cls");
				break;
			}
			case 4:
			{
				print_type_in_value_for("alfa_2");
				std::cin >> config.alfa_2;
				system("cls");
				break;
			}
			case 5:
			{
				print_type_in_value_for("N");
				std::cin >> config.N;
				system("cls");
				break;
			}
			case 6:
			{
				print_type_in_value_for("Main loop itarations");
				std::cin >> config.main_loop_itarations;
				system("cls");
				break;
			}
			case 7:
			{
				print_type_in_value_for("Single route improvement margin");
				std::cin >> config.single_route_improvement_margin;
				system("cls");
				break;
			}
			default:
			{
				std::cout << "\nNieporawna wartosc";
			}

		}


	}
}

void Ui::set_hybrid_params(hybridAvnsLnsConfig& config)
{
	bool run = true;
	while (run)
	{
		system("cls");
		std::cout << "\n======= Aktualne wartosci konfiguracyjne Hybrid of AVNS and LNS =======\n";
		std::cout << "\n1. maxDiv (130) = " << config.maxDiv << "		Liczba iteracji etapu pierszego AVNS";
		std::cout << "\n2. maxDiv2 (110) = " << config.maxDiv2 << "		Liczba iteracji etapu drugiego AVNS";
		std::cout << "\n0. Wyjscie\n";

		std::cout << "Wybierz opcje: ";
		int choice;
		std::cin >> choice;
		std::cin.ignore(10000, '\n');
		switch (choice)
		{
		case 0:
		{
			run = false;
			break;
		}
		case 1:
		{
			print_type_in_value_for("maxDiv");
			std::cin >> config.maxDiv;
			system("cls");
			break;
		}
		case 2:
		{
			print_type_in_value_for("maxDiv2");
			std::cin >> config.maxDiv2;
			system("cls");
			break;
		}	
		{
			std::cout << "\nNieporawna wartosc";
		}

		}


	}
}

void Ui::set_skewed_params(SkewedVNSConfig& config)
{
	bool run = true;
	while (run)
	{
		system("cls");
		std::cout << "\n======= Aktualne wartosci konfiguracyjne Skewed general variable neighborhood search =======\n";
		std::cout << "\n1. f_alfa (500) = " << config.f_alfa << "		Waga kary za przekroczenie naruszenia pojemnosci pojazdu";
		std::cout << "\n2. SVNS_max_no_improve (300) = " << config.SVNS_max_no_improve << "		Liczba iteracji glownego algorytmu bez znalezienia poprawy";
		std::cout << "\n3. alfa (5) = " << config.delta_alfa << "		Wspolczynnik tolerancji gorszych rozwiazan (skewed move)";
		std::cout << "\n0. Wyjscie\n";

		std::cout << "Wybierz opcje: ";
		int choice;
		std::cin >> choice;
		std::cin.ignore(10000, '\n');
		switch (choice)
		{
		case 0:
		{
			run = false;
			break;
		}
		case 1:
		{
			print_type_in_value_for("f_alfa");
			std::cin >> config.f_alfa;
			system("cls");
			break;
		}
		case 2:
		{
			print_type_in_value_for("SVNS_max_no_improve");
			std::cin >> config.SVNS_max_no_improve;
			system("cls");
			break;
		}
		case 3:
		{
			print_type_in_value_for("alfa");
			std::cin >> config.delta_alfa;
			system("cls");
			break;
		}
		{
			std::cout << "\nNieporawna wartosc";
		}

		}


	}
}



void Ui::set_number_of_runs()
{
	bool run = true;
	while (run)
	{
		system("cls");
		std::cout << "\n======= Liczba uruchomien metod =======\n";
		std::cout << "\n1. Brain Storm Optimalization = " << number_bso;
		std::cout << "\n2. Hybrid of AVNS and LNS  = " << number_hybrid;
		std::cout << "\n3. Skewed general variable neighborhood search = " << number_skewed;
		std::cout << "\n0. Wyjscie\n";

		std::cout << "\nWybierz opcje: ";
		int choice;
		std::cin >> choice;
		std::cin.ignore(10000, '\n');
		switch (choice)
		{
		case 0:
		{
			run = false;
			break;
		}
		case 1:
		{
			print_type_in_value_for("Liczba uruchomien BSO");
			std::cin >> number_bso;
			system("cls");
			break;
		}
		case 2:
		{
			print_type_in_value_for("Liczba uruchomien hybrid");
			std::cin >> number_hybrid;
			system("cls");
			break;
		}
		case 3:
		{
			print_type_in_value_for("Liczba uruchomien Skewed");
			std::cin >> number_skewed;
			system("cls");
			break;
		}
		default:
		{
			std::cout << "\nNieporawna wartosc";
		}

		}
	}
}

void Ui::set_input_path()
{
	bool run = true;
	while (run)
	{
		system("cls");
		std::cout << "\n======= Wybor sciezki problemu =======\n";
		std::cout << "\nAktualna sciezka: " << input_path + instance_name ;
		std::cout << "\n\n1. Sciezka pliku";
		std::cout << "\n2. Nazwa pliku";
		std::cout << "\n3. Liczba pojazdow";
		std::cout << "\n0. Wyjscie";
		std::cout << "\nWybierz opcje: ";
		int choice;
		std::cin >> choice;
		std::cin.ignore(10000, '\n');
		switch (choice)
		{
		case 0:
		{
			run = false;
			break;
		}
		case 1:
		{
			system("cls");
			set_full_path_input();
			break;
		}
		case 2:
		{
			system("cls");
			set_instance_name();
			break;
		}
		case 3:
		{
			std::cout << "\n Aktualna liczba pojazdow: " << num_vehicles;
			print_type_in_value_for("liczby pojazdow");
			std::cin >> num_vehicles;
			system("cls");
			break;
		}
		default:
		{
			std::cout << "\nNieporawna wartosc";
		}
		}
	}
}


void Ui::set_full_path_input()
{
	
	input_path = get_line_with_default("Sciezka:   ", input_path);

}

void Ui::set_instance_name()
{
	instance_name = get_line_with_default("Nazwa pliku: ", instance_name);
}

void Ui::set_result_path()
{
	bool run = true;
	while (run)
	{
		system("cls");
		std::cout << "\n======= Wybor sciezki zapisu =======\n";
		std::cout << "\nAktualna sciezka: " << result_path;
		std::cout << "\nFolder zapisu: " << result_folder;
		std::cout << "\n\n1. Sciezka";
		std::cout << "\n2. Folder";
		std::cout << "\n0. Wyjscie";
		std::cout << "\nWybierz opcje: ";
		int choice;
		std::cin >> choice;
		std::cin.ignore(10000, '\n');
		switch (choice)
		{
		case 0:
		{
			run = false;
			break;
		}
		case 1:
		{
			system("cls");
			result_path = get_line_with_default("Sciezka: ", result_path);
			break;
		}
		case 2:
		{
			system("cls");
			result_folder = get_line_with_default("Folder: ", result_folder);
			break;
		}
		default:
		{
			std::cout << "\nNieporawna wartosc";
		}
		}
	}
}

void Ui::print_type_in_value_for(std::string param)
{
	std::cout << "\nPodaj nowa wartosc dla " << param << ":";
}

std::string Ui::get_line_with_default(const std::string& prompt, const std::string& defaultValue)
{
	std::cout << prompt;

	// Pobieramy uchwyt do standardowego wejœcia konsoli
	HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);

	// Przygotowujemy zdarzenia klawiatury dla ka¿dego znaku w defaultValue
	std::vector<INPUT_RECORD> inputRecords(defaultValue.length());
	for (size_t i = 0; i < defaultValue.length(); ++i) {
		inputRecords[i].EventType = KEY_EVENT;
		inputRecords[i].Event.KeyEvent.bKeyDown = TRUE;
		inputRecords[i].Event.KeyEvent.uChar.UnicodeChar = defaultValue[i];
		inputRecords[i].Event.KeyEvent.wRepeatCount = 1;
		inputRecords[i].Event.KeyEvent.wVirtualKeyCode = 0; // Niepotrzebne dla UnicodeChar
		inputRecords[i].Event.KeyEvent.dwControlKeyState = 0;
	}

	// Wstrzykujemy znaki do bufora konsoli
	DWORD written;
	WriteConsoleInput(hInput, inputRecords.data(), (DWORD)inputRecords.size(), &written);

	// Teraz standardowy getline "pomyœli", ¿e u¿ytkownik sam to wpisa³
	std::string userInput;
	std::getline(std::cin, userInput);

	return userInput;
}