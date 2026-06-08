# Implementacja algorytmów metaheurystycznych dla problemu CCVRP

Repozytorium zawiera implementację trzech algorytmów metaheurystycznych przeznaczonych do rozwiązywania kumulacyjnego problemu trasowania pojazdów z ograniczeniami pojemnościowymi, czyli **CCVRP** (*Capacitated Cumulative Vehicle Routing Problem*).

Zaimplementowane metody zostały odtworzone na podstawie prac literaturowych i porównane eksperymentalnie na wybranych instancjach testowych ze zbioru Golden et al. Projekt powstał jako część pracy dyplomowej dotyczącej implementacji i analizy algorytmów dla problemu CCVRP.

## Opis problemu

Problem CCVRP jest odmianą klasycznego problemu trasowania pojazdów z ograniczeniami pojemnościowymi. Celem jest wyznaczenie tras pojazdów obsługujących wszystkich klientów przy zachowaniu ograniczeń pojemnościowych oraz minimalizacji kumulacyjnego kosztu obsługi.

W odróżnieniu od klasycznego CVRP, w którym najczęściej minimalizowana jest całkowita długość tras, w CCVRP istotna jest również kolejność odwiedzania klientów. Koszt rozwiązania zależy od czasu lub odległości pokonywanej przed obsługą kolejnych klientów.

## Zaimplementowane algorytmy

W projekcie zaimplementowano trzy metody metaheurystyczne:

- **AVNS--LNS** — algorytm hybrydowy łączący adaptacyjne przeszukiwanie ze zmiennym sąsiedztwem (*Adaptive Variable Neighborhood Search*) oraz przeszukiwanie dużego sąsiedztwa (*Large Neighborhood Search*),
- **SGVNS** — algorytm skośnego uogólnionego przeszukiwania ze zmiennym sąsiedztwem (*Skewed General Variable Neighborhood Search*),
- **BSO** — algorytm optymalizacji burzy mózgów (*Brain Storm Optimization*).

Każda metoda korzysta ze wspólnej reprezentacji rozwiązania, formatu danych wejściowych oraz mechanizmu zapisu wyników, co umożliwia ich porównanie w jednolitych warunkach eksperymentalnych.

W projekcie nie korzysta się z dodatkowych bibliotek realizujących jakiekolwiek operacje związane z trasowaniem. 
Jedyną gotową biblioteką w projecie jest **NLOHMANN JSON**. 

## Funkcjonalności programu

Program umożliwia:

- wczytywanie instancji problemu z plików `.vrp`,
- uruchamianie algorytmów AVNS--LNS, SGVNS oraz BSO,
- konfigurację parametrów algorytmów z poziomu pliku `Settings.JSON`, lub interfejsu konsolowego,
- wykonywanie wielu niezależnych uruchomień dla tej samej instancji,
- zapis końcowych wyników działania algorytmów,
- zapis przebiegu zmian wartości funkcji celu w trakcie działania metod,
- porównywanie wyników uzyskanych przez różne algorytmy.
