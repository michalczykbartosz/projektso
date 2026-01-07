#pragma once
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "wspolne.h"

class kolejka
{
private:
	int id_kolejka; //systemowe id kolejki komunikatow
	bool czy_wlasciciel; //flaga sterujaca usuwaniem zasobu w destruktorze

public:
	kolejka(bool wlasciciel = false); //konstruktor tworzacy kolejke lub pobiera istniejaca
	~kolejka(); //destruktor niszczacy kolejke jesli proces jest wlascicielem

	void wyslij(int typ,int id_nadawcy,const char* tekst); /*funkcja wysylajaca sformatowany komunikat do kolejki: typ - kanał(1: logi, 4 : ekspres)
	tekst - tresc wiadomosci*/
	
	komunikat odbierz(int typ = 0); //funkcja odbierajaca komunikat w trybie blokujacym (proces jest usypiany az nadejdzie wiadomosc)

	int odbierz_nieblokujaco(int typ, komunikat& msg); /*funkcja odbierajaca komunikat w trybie nieblokujacym uzywajaca flage IPC_NOWAIT
	tak aby ciezarowki mogly sprawdzac czy jest pakiet ekspresowy bez zatrzymywania pracy*/

	

};



