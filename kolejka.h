#pragma once
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "wspolne.h"

class kolejka
{
private:
	int id_kolejka; //systemowe id kolejki komunikatow
	int pid_dyspozytora; //pid procesu glownego, tylko proces z tym pidem moze wywolac destruktor

public:
	kolejka(); //konstruktor tworzacy kolejke lub pobiera istniejaca
	~kolejka(); //destruktor niszczacy kolejke jesli biezacy proces to pid_dyspozytora

	void wyslij(int typ,int id_nadawcy,const char* tekst); /*funkcja wysylajaca sformatowany komunikat do kolejki: typ - kanał(1: logi, 4 : ekspres)
	tekst - tresc wiadomosci*/
	
	komunikat odbierz(int typ = 0); //funkcja odbierajaca komunikat w trybie blokujacym (proces jest usypiany az nadejdzie wiadomosc)

	int odbierz_nieblokujaco(int typ, komunikat& msg); /*funkcja odbierajaca komunikat w trybie nieblokujacym uzywajaca flage IPC_NOWAIT
	tak aby ciezarowki mogly sprawdzac czy jest pakiet ekspresowy bez zatrzymywania pracy*/

	

};



