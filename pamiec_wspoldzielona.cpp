#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <cstdlib>
#include <unistd.h>
#include <cstdio>
#include "wspolne.h"
#include "pamiec_wspoldzielona.h"

shared_memory::shared_memory(bool wlasciciel) //konstruktor pamieci wspoldzielonej
{
	czy_wlasciciel = wlasciciel;
	key_t klucz;// = 11111;
	klucz = ftok(".", 'P'); //utworzenie klucza do pamieci wspoldzielonej

	if (klucz == -1)// wypisanie bledu jesli nie udalo sie stworzyc klucza
	{
		perror("Blad ftok() dla pamieci wspoldzielonej");
		exit(1);
	}

	id_pamieci = shmget(klucz, sizeof(stan_tasmy), IPC_CREAT | 0600); //utworzenie pamieci wspol dzielonej z prawami 0666

	if (id_pamieci == -1)
	{
		perror("Blad tworzenia pamieci wspoldzielonej!");
		exit(1);
	}

	adres = (stan_tasmy*) shmat(id_pamieci, nullptr,0); //dolaczenie pamieci wspoldzielonej

	if (adres == (void*)-1) //wypisanie bledu jesli nie udalo sie dolaczyc pamieci wspoldzielonej
	{
		perror("Blad dolaczania pamieci wspoldzielonej!");
		exit(1);

		if (czy_wlasciciel)//usuniecie pamieci jesli nie udalo sie jej dolaczyc
		{
			shmctl(id_pamieci, IPC_RMID, NULL);
		}
		exit(1);
	}
}


shared_memory::~shared_memory() //destruktor ktory usuwa pamiec wspoldzielona gdy dyspozytor (wlasciciel) konczy prace
{
	if (shmdt(adres) == -1)//odlaczenie segmentu pamieci systemowej (kazdy proces)
	{
		perror("Blad odlaczania pamieci wspoldzielonej");
	}

	if (czy_wlasciciel)
	{
		if (shmctl(id_pamieci, IPC_RMID, NULL) == -1)
		{
			perror("Blad usuwania pamieci wspoldzielonej");
		}
		else
		{
			printf("Poprawnie usunieto pamiec wspoldzielona\n");
		}
	}

}

stan_tasmy* shared_memory::dane() //metoda dane ktora zwraca adres pamieci wspoldzielonej
{
	return adres;
}

void shared_memory::zapisz(int id_pracownika,char typ, double waga)
{
	int indeks = adres->tail;
	adres->bufor[indeks].id_pracownika = id_pracownika;
	adres->bufor[indeks].typ = typ;
	adres->bufor[indeks].waga = waga;
	adres->bufor[indeks].priorytet = false;

	adres->tail = (adres->tail + 1) % MAX_PACZEK;
	adres->aktualna_liczba_paczek++;
	adres->aktualna_waga_paczek_tasma += waga;
}