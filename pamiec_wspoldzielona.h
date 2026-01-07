#pragma once
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include "wspolne.h"


class shared_memory
{
private:
	int id_pamieci;
	stan_tasmy* adres;
	bool czy_wlasciciel;

public:
	shared_memory(bool wlasciciel = false);
	~shared_memory();
	stan_tasmy* dane();

	void zapisz(int id_pracownika,char typ,double waga);
	Paczka odczytaj();
};
