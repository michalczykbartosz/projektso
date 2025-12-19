#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <cstdio>
#include "wspolne.h"
#include "pamiec_wspoldzielona.h"

shared_memory::shared_memory() //konstruktor pamieci wspoldzielonej
{
	key_t klucz;
	klucz = ftok(".", 'P'); //utworzenie klucza do pamieci wspoldzielonej

	id_pamieci = shmget(klucz, sizeof(stan_tasmy), IPC_CREAT | 0666); //utworzenie pamieci wspol dzielonej z prawami 0666

	if (id_pamieci == -1)
	{
		perror("Blad tworzenia pamieci wspoldzielonej!");
		exit(1);
	}

	adres = (stan_tasmy*) shmat(id_pamieci, nullptr,0); //dolaczenie pamieci wspoldzielonej

	if (adres == (void*)-1)
	{
		perror("Blad dolaczania pamieci wspoldzielonej!");
		exit(1);
	}

	pid_dyspozytora = getpid();
}


shared_memory::~shared_memory() //destruktor ktory usuwa pamiec wspoldzielona gdy dyspozytor (wlasciciel) konczy prace
{
	shmdt(adres); //odlaczenie segmentu pamieci systemowej (kazdy proces)

	if (getpid() == pid_dyspozytora)
	{
		shmctl(id_pamieci,IPC_RMID,nullptr);
	}

}

stan_tasmy* shared_memory::dane() //metoda dane ktora zwraca adres pamieci wspoldzielonej
{
	return adres;
}