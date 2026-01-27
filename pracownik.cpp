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
#include <sched.h> 

kolejka* globalna_kolejka = nullptr; //utworzenie zasobu globalnej kolejki
bool czy_pracowac = true; //flaga sterujaca glowna petla pracownikow

/*
//funkcja ktora sygnalizuje koniec pracy po przyjeciu sygnalu SIGINT
void obsluga_konca(int sig)
{
	czy_pracowac = false;
}
*/

//funkcja pomocnicza do losowania wagi paczki
double losuj_wage(double min, double max) 
{
	double f = (double)rand() / RAND_MAX;
	return min + f * (max - min);
}

//funkcja obslugujaca sygnaly dla pracownika ekspresowego P4
void obsluga_P4(int sygnal) 
{
	if (sygnal == SIGUSR2) //jesli sygnal to SIGUSR2 pracownik losuje wage i laduje pakiet ekspresowy
	{
		loguj(P4,"Pracownik P4: Otrzymyalem sygnal 2 - Laduje pakiet ekspersowy\n");
		double waga = losuj_wage(0.1, 24.9);
		loguj(P4,"Pracownik P4: Zaladowano pakiet (%.2f kg) do ciezarowki\n", waga);

		if (globalna_kolejka != nullptr) //jesli globalna kolejka poprawnie dziala, wysylamy pakiet ekspresowy za pomoca kolejki komunikatow
		{
			char txt[20];
			sprintf(txt, "%f", waga);
			globalna_kolejka->wyslij(4,4, txt);
		}
	}

	else if (sygnal == SIGTERM || sygnal == SIGINT) //po wyslaniu sygnalu konca pracownik P4 konczy prace
	{
		loguj(P4,"Pracownik P4 zakonczyl prace\n");
		exit(0);
	}
}

void obsluga_pracownikow(int sygnal) //po wyslaniu sygnalu konca zwykli pracownicy koncza prace
{
	if (sygnal == SIGTERM || sygnal == SIGINT)
	{
		czy_pracowac = false;
	}
}


int main(int argc, char* argv[])
{
	if (argc < 2) bledy::rzuc_blad(4); //jezeli jest mniej niz 2 argumenty, program rzuca blad
	int id = atoi(argv[1]); //konwersja tekstu na liczbe int
	srand(time(NULL) ^ (getpid() << 16)); //inicjalizacja seedu generatora liczb pseudolosowych
	//inicjalizacja zasobow - semafory, kolejka komunikatow oraz pamiec wspoldzielona
	semafor sem(5);
	shared_memory pamiec;
	kolejka kol;
	globalna_kolejka = &kol;
	//signal(SIGINT, obsluga_konca); //zarejestrowanie sygnalu konca pracy

	if (id == 4) //jesli id == 4 jest to pracownik ekspresowy P4
	{
		loguj(P4,"Pracownik P4 (Ekspresowy) czeka na sygnaly\n");
		
		//rejestrowanie sygnalow konca dla pracownika ekspresowego P4
		signal(SIGUSR2, obsluga_P4);
		signal(SIGTERM, obsluga_P4);
		signal(SIGINT, obsluga_P4);

		while (true) //petla oczekiwania dla pracownika ekspresowego P4
		{
			pause();
		}
		return 0;
	}

	//rejestracja sygnalow konca dla zwyklych pracownikow
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

	while (czy_pracowac) //glowna petla pracy zwyklych pracownikow
	{
		//sleep(1);
		if (!czy_pracowac) break; //jesli czy_pracowac == false od razu przerywamy petle

		while (true) //petla sprawdzajaca pauze
		{
			sem.p(0);
			bool paused = pamiec.dane()->system_paused;
			sem.v(0);

			if (!paused) break; //brak pauzy - kontynuuj

			//sched_yield(); //oddaj cpu
		}

		double waga = losuj_wage(min_waga, max_waga); //losowanie wagi
		bool czy_dzwignie = false; //ustawienie flagi udzwigu tasmy

		while (!czy_dzwignie) //petla sprawdzajaca czy tasma udzwignie paczke
		{
			sem.p(1); //rezerwacja wolnego miejsca na tasmie 
			sem.p(0); //wejscie do sekcji krytycznej, zablokowanie pamieci
			stan_tasmy* tasma = pamiec.dane();

			if (tasma->aktualna_waga_paczek_tasma + waga <= UDZWIG) //sprawdzenie czy wszystkie paczki na tasmie + nowa paczka na tasmie <= UDZWIG
			{
				pamiec.zapisz(id,typ_paczki,waga); //zapisanie paczki do pamieci wspoldzielonej
				loguj(PRACOWNIK,"Pracownik P%d: Dodalem %c (%.2f kg). Tasma: %d szt, %.1f/%.0f kg\n",
					id, typ_paczki, waga, pamiec.dane()->aktualna_liczba_paczek, pamiec.dane()->aktualna_waga_paczek_tasma, (double)UDZWIG);

				sem.v(0); //wyjscie z sekcji krytycznej, odblokowanie pamieci
				sem.v(2); //na tasmie pojawila sie nowa paczka, ciezarowka moze ja odebrac
				czy_dzwignie = true; //zmiana flagi ze paczka zmiesci sie na tasmie
				kol.wyslij(1,id, "Dodano paczke"); //wyslanie komunikatu o dodaniu paczki
			}
			else //jezeli paczka sie nie zmiesci na tasmie podnosimy semafory tak zeby nie zblokowac produkcji
			{
				sem.v(0); //odblokowanie pamieci zeby praca mogla dalej trwac
				sem.v(1); //zwrot zarezerwowanego miejsca na tasmie
				//sleep(1);
			}
		}
			

	}

}

