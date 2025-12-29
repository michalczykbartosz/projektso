#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <ctime>
#include "semafory.h"
#include "pamiec_wspoldzielona.h"
#include "kolejka.h"
#include "bledy.h"
#include <csignal>
#include "logger.h"

kolejka* globalna_kolejka = nullptr;
bool czy_pracowac = true;

void obsluga_konca(int sig)
{
	czy_pracowac = false;
}

double losuj_wage(double min, double max) //funkcja pomocnicza do losowania wagi paczki
{
	double f = (double)rand() / RAND_MAX;
	return min + f * (max - min);
}

void obsluga_P4(int sygnal) //funkcja do obslugi sygnalow dla  pracownika ekspresowego
{
	if (sygnal == SIGUSR2)
	{
		loguj(P4,"Pracownik P4: Otrzymyalem sygnal 2 - Laduje pakiet ekspersowy\n");
		double waga = losuj_wage(0.1, 24.9);
		loguj(P4,"Pracownik P4: Zaladowano pakiet (%.2f kg) do ciezarowki\n", waga);

		if (globalna_kolejka != nullptr)
		{
			char txt[20];
			sprintf(txt, "%f", waga);
			globalna_kolejka->wyslij(4,4, txt);
		}
	}

	else if (sygnal == SIGTERM || sygnal == SIGINT)
	{
		loguj(P4,"Pracownik P4 zakonczyl prace\n");
		exit(0);
	}
}

void obsluga_pracownikow(int sygnal)
{
	if (sygnal == SIGTERM || sygnal == SIGINT)
	{
		czy_pracowac = false;
	}
}


int main(int argc, char* argv[])
{
	if (argc < 2) bledy::rzuc_blad(4); //jezeli jest mniej niz 2 argumenty, program sie nie uruchomi
	int id = atoi(argv[1]);
	srand(time(NULL) ^ (getpid() << 16));
	//inicjalizacja zasobow - semafory, kolejka komunikatow oraz pamiec wspoldzielona
	semafor sem(4);
	shared_memory pamiec;
	kolejka kol;
	globalna_kolejka = &kol;
	signal(SIGINT, obsluga_konca);

	if (id == 4) //jesli argumentem jest 4, czyli jest to pracownik ekspresowy 
	{
		loguj(P4,"Pracownik P4 (Ekspresowy) czeka na sygnaly\n");
		
		signal(SIGUSR2, obsluga_P4);
		signal(SIGTERM, obsluga_P4);
		signal(SIGINT, obsluga_P4);

		while (true)
		{
			pause();
		}
		return 0;
	}

	signal(SIGTERM, obsluga_pracownikow);
	signal(SIGINT, obsluga_pracownikow);

	char typ_paczki;
	double min_waga, max_waga;

	//ustalenie danych paczki - zaleznie od id czytanego z argumentow
	switch (id)
	{
	case 1:
		typ_paczki = 'A';
		min_waga = 0.1; max_waga = 8.0;
		break;
	case 2:
		typ_paczki = 'B';
		min_waga = 8.1; max_waga = 16.0;
		break;
	case 3:
		typ_paczki = 'C';
		min_waga = 16.1; max_waga = 25.0;
		break;
	default:
		typ_paczki = 'A'; min_waga = 1.0; max_waga = 5.0;
		break;
	}

	loguj(PRACOWNIK,"Pracownik P%d: Zaczynam produkcje paczek typu: %c\n", id, typ_paczki);

	while (czy_pracowac)
	{
		sleep(3);
		if (!czy_pracowac) break;

		double waga = losuj_wage(min_waga, max_waga);
		bool czy_dzwignie = false;

		while (!czy_dzwignie)
		{
			sem.p(1);
			sem.p(0);
			stan_tasmy* tasma = pamiec.dane();

			if (tasma->aktualna_waga_paczek_tasma + waga <= UDZWIG)
			{
				pamiec.zapisz(id,typ_paczki,waga);
				loguj(PRACOWNIK,"Pracownik P%d: Dodalem %c (%.2f kg). Tasma: %d szt, %.1f/%.0f kg\n",
					id, typ_paczki, waga, pamiec.dane()->aktualna_liczba_paczek, pamiec.dane()->aktualna_waga_paczek_tasma, (double)UDZWIG);

				sem.v(0);
				sem.v(2);
				czy_dzwignie = true;
				kol.wyslij(1,id, "Dodano paczke");
			}
			else
			{
				sem.v(0);
				sem.v(1);
				sleep(1);
			}
		}
			

	}

}

