#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <ctime>
#include "semafory.h"
#include "pamiec_wspoldzielona.h"
#include "kolejka.h"
#include "bledy.h"
#include <csignal>

bool czy_pracowac = true;
bool wymuszony_odjazd = false;

void pracowanie(int sygnal)
{
	
	if (sygnal == SIGTERM || sygnal == SIGINT)
	{
		czy_pracowac = false;
	}
}

void ekspresowy_odjazd(int sygnal)
{
	if (sygnal == SIGUSR1)
	{
		wymuszony_odjazd = true;
	}
}

int main(int argc, char* argv[])
{
	semafor sem(4);
	shared_memory pam;
	kolejka kol;

	signal(SIGTERM, pracowanie);
	signal(SIGINT, pracowanie);
	signal(SIGUSR1, ekspresowy_odjazd);


	while (czy_pracowac)
	{
		sem.p(3);
		printf("Ciezarowka przy rampie\n");
		bool pelna = false;
		wymuszony_odjazd = false;
		double waga_ciezarowki = 0.0;
		double objetosc_ciezarowki = 0.0;


		while (!pelna && czy_pracowac)
		{
			struct komunikat msg;
			if (wymuszony_odjazd == true)
			{
				break;
			}

			if (kol.odbierz_nieblokujaco(4, msg) != -1)
			{
				double waga_ekspresowych = atof(msg.text);
				double objetosc_ekspresowych = 0.5;

				if (waga_ciezarowki + waga_ekspresowych <= MAX_WAGA_CIEZAROWKA && objetosc_ciezarowki + objetosc_ekspresowych <= MAX_OBJETOSC_CIEZAROWKA)
				{
					waga_ciezarowki += waga_ekspresowych;
					objetosc_ciezarowki += objetosc_ekspresowych;
					printf("CIEZAROWKA: Zaladaowano pakiet ekspres %.2f - waga calej ciezarowki: %.2f", waga_ekspresowych, waga_ciezarowki);
				}

			}
			sem.p(2);
			sem.p(0);

			stan_tasmy* st = pam.dane();
			Paczka p = st->bufor[st->head];

			if (p.waga < 0.001)
			{
				sem.v(0);
				sem.v(2); 
				continue; 
			}

			double obj = 0.0;
			if (p.typ == 'A') obj = 0.2432 * 0.08;
			else if (p.typ == 'B') obj = 0.2432 * 0.19;
			else obj = 0.2432 * 0.41;

			if (waga_ciezarowki + p.waga <= MAX_WAGA_CIEZAROWKA && objetosc_ciezarowki + obj <= MAX_OBJETOSC_CIEZAROWKA)
			{
				waga_ciezarowki += p.waga;
				objetosc_ciezarowki += obj;
				st->bufor[st->head].waga = 0.0;
				st->aktualna_liczba_paczek--;
				st->aktualna_waga_paczek_tasma -= p.waga;
				if (st->aktualna_liczba_paczek <= 0)
				{
					st->aktualna_liczba_paczek = 0;
					st->aktualna_waga_paczek_tasma = 0.0;
				}
				st->head = (st->head + 1) % MAX_PACZEK;

				printf("CIEZAROWKA: Paczka %c (%.2f). Stan ciezarowki: %.2f/%.2f\n", p.typ, p.waga, waga_ciezarowki, (double)MAX_WAGA_CIEZAROWKA);

				sem.v(0);
				sem.v(1); 
			}
			else
			{
				pelna = true;
				sem.v(0);
				sem.v(2);
			}
		}
		printf("CIEZARKOWA: Odjezdzam z waga %.2f\n", waga_ciezarowki);
		sem.v(3);
		if (czy_pracowac) sleep(10);
			
return 0;
	}
}



	
