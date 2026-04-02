#pragma once


//informuje nas o tym jakiego klienta przeniesc na jaka pozycje i z jakim zyskiem
//z zalozenia im wiekszy zysk tym lepszy ruch, ale to zalezy od uzytej metody do wyznaczania gaina
struct Move {
	int from_route, from_pos;
	int to_route, to_pos;	
	int number_of_moved_clients; 
	double gain;
	int moved_capacity_from; // ile pojemnosci zostanie zabrane z from_route w wyniku tego ruchu - czyli tyle zostanie dodane do trasy to_route
	int moved_capacity_to; // ile pojemnosci zostanie zabrane z to_route i dodane do from_route
	//te pojemnosci to w zzaleznosci od tego czy prznosimy jednego klienta czy dwoch, w obrebie jakich tras itp
};