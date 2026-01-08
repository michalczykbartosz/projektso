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

//inicjalizacja globlanych flag sterujacych procesem
bool czy_pracowac = true; //flaga glownej petli 
bool wymuszony_odjazd = false; //flaga ustawiana przez SIGUSR1
bool przy_rampie = false; //flaga informujaca czy ciezarowka jest przy rampie

//funkcja sprawdzajaca czy nie przyszedl sygnal SIGTERM lub SIGINT ktory konczy prace ciezarowki
void pracowanie(int sygnal) 
{
	
	if (sygnal == SIGTERM || sygnal == SIGINT)
	{
		czy_pracowac = false; //koniec pracy po zakonczeniu cyklu
	}
}

//funkcja sprawdzajaca czy nie przyszedl sygnal SIGUSR1 po ktorym powinna odjechac natychmiast (nawet nie pelna)
void ekspresowy_odjazd(int sygnal) 
{
	if (sygnal == SIGUSR1 && przy_rampie)
	{
		if (przy_rampie)
		{
			wymuszony_odjazd = true; //flaga przerywajaca petle ladowania
		}
	}
}

int main(int argc, char* argv[])
{
	//inicjalizacja zasobow systemowych
	semafor sem(4);
	shared_memory pam;
	kolejka kol;

	//rejestracja sygnalow
	signal(SIGTERM, pracowanie);
	signal(SIGINT, pracowanie);
	signal(SIGUSR1, ekspresowy_odjazd);

	stan_tasmy* st = pam.dane(); //wskaznik na strukture w pamieci wspoldzielonej

	//glowna petla pracy ciezarowki, dziala dopoki jest flaga lub na tasmie sa paczki
	while (czy_pracowac || st->aktualna_liczba_paczek > 0) 
	{
		//podjazd pod rampe
		sem.p(3); //opuszczenie semafora (semafor nr. 3 kontroluje czy jest ciezarowka przy rampie)
		przy_rampie = true;
		loguj(CIEZAROWKA,"Ciezarowka przy rampie\n");
		//resetowanie zmiennych dla nowej ciezaorwki
		bool pelna = false;
		wymuszony_odjazd = false;
		double waga_ciezarowki = 0.0;
		double objetosc_ciezarowki = 0.0;

		//petla ladowania, dziala dopoki jest miejsce i nie ma rozkazu odjazdu
		while (!pelna) 
		{
			struct komunikat msg;

			//odjazd ciezarowki po sygnale SIGUSR1
			if (wymuszony_odjazd == true) 
			{
				break; //przerwanie ladowania, ciezarowka odjezdza z tym co juz zaladowane
			}

			
			sem.p(0); //blokowanie mutexu aby bezpiecznie odczytac stan tasmy (wejscie do sekcji krytycznej pamieci)
			//sprawdzenie czy magazyn zakonczyl prace i czy tasma jest pusta
			if (!czy_pracowac && st->aktualna_liczba_paczek == 0) 
			{
				sem.v(0); //wyjscie z sekcji krytycznej pamieci 
				break; //koniec pracy i odjazd
			}
			sem.v(0); //zwolnienie mutexu po sprawdzeniu

			//obsluga pakietow ekspresowych uzywa IPC_NOWAIT aby nie czekala na pakiety ekspresowe gdy ich nie ma
			if (kol.odbierz_nieblokujaco(4, msg) != -1) 
			{
				double waga_ekspresowych = atof(msg.text); //konwersja tekstu na liczbe
				double objetosc_ekspresowych = 0.2; //stala objetosc ekspresowych

				//sprawdzenie czy pakiet ekspresowy sie zmiesci do ciezarowki
				if (waga_ciezarowki + waga_ekspresowych <= MAX_WAGA_CIEZAROWKA && objetosc_ciezarowki + objetosc_ekspresowych <= MAX_OBJETOSC_CIEZAROWKA)
				{
					waga_ciezarowki += waga_ekspresowych;
					objetosc_ciezarowki += objetosc_ekspresowych;
					loguj(CIEZAROWKA,"CIEZAROWKA: Zaladaowano pakiet ekspres %.2f - waga calej ciezarowki: %.2f\n", waga_ekspresowych, waga_ciezarowki);
				}
				//jesli pakiet sie nie miesci do ciezarowki, natychmiastowy odjazd i podstawienie nowej zeby zaladowac pakiet
				else
				{
					kol.wyslij(4, msg.id_nadawcy, msg.text); //wyslanie komunikatu
					loguj(CIEZAROWKA, "CIEZAROWKA: Ekspres %.2f sie nie zmiescil, wymuszam odjazd\n",waga_ekspresowych); 
					pelna = true; //wymuszenie odjazdu
				}

			}
			//obsluga paczek standardowo
			//sem.p(2); //oczekiwanie na paczke na tasmie
			if (sem.p_przerywalne(2) == false)
			{
				if (wymuszony_odjazd) //jezeli funkcja zwraca false znaczy ze przerwal ja sygnal
				{
					break; //przerwanie petli ladowania i odjazd
				}
				continue; //jesli to inny sygnal, probujemy dalej
			}
			sem.p(0); //mutex pamieci

			Paczka p = st->bufor[st->head];

			//zabezpieczenie przed pustymi paczkami
			if (p.waga < 0.001) 
			{
				sem.v(0); //odblokowanie pamieci
				sem.v(2); //oddanie semaforu nr. 2 poniewaz nie wzielismy paczki
				continue; 
			}

			double obj = 0.0;
			if (p.typ == 'A') obj = 0.2432 * 0.08;
			else if (p.typ == 'B') obj = 0.2432 * 0.19;
			else obj = 0.2432 * 0.41;

			//sprawdzenie czy paczka z tasmy zmiesci sie w ciezarowce
			if (waga_ciezarowki + p.waga <= MAX_WAGA_CIEZAROWKA && objetosc_ciezarowki + obj <= MAX_OBJETOSC_CIEZAROWKA)
			{
				waga_ciezarowki += p.waga;
				objetosc_ciezarowki += obj;
				//aktualizacja stanu tasmy
				st->bufor[st->head].waga = 0.0;
				st->aktualna_liczba_paczek--;
				st->aktualna_waga_paczek_tasma -= p.waga;
				//zabezpieczenie ujemnych danych
				if (st->aktualna_liczba_paczek <= 0) 
				{
					st->aktualna_liczba_paczek = 0;
					st->aktualna_waga_paczek_tasma = 0.0;
				}
				st->head = (st->head + 1) % MAX_PACZEK; //akutalizacja wskaznika odczytu

				loguj(CIEZAROWKA,"CIEZAROWKA: Paczka %c (%.2f). Stan ciezarowki: %.2f/%.2f\n", p.typ, p.waga, waga_ciezarowki, (double)MAX_WAGA_CIEZAROWKA);

				sem.v(0); //odblokowanie pamieci
				sem.v(1); //zasygnalizowanie wolnego miejsca na tasmie
			}
			//brak miejsca
			else
			{
				pelna = true;
				sem.v(0); //odblokowanie pamieci
				sem.v(2); //oddanie semafora, paczka zostanie na tasmie dla nastepnej ciezarowki
			}
		}
		//odjazd ciezarowki
		loguj(CIEZAROWKA,"CIEZARKOWA: Odjezdzam z waga %.2f\n", waga_ciezarowki);
		przy_rampie = false;
		sem.v(3); //zwolnienie rampy
		if (czy_pracowac) sleep(CZAS_JAZDY); //symulacja podrozy ciezarowki
			
	}
	return 0;
}



	
