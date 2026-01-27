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

//inicjalizacja globlanych flag sterujacych procesem
bool czy_pracowac = true; //flaga glownej petli 
bool wymuszony_odjazd = false; //flaga ustawiana przez SIGUSR1
bool przy_rampie = false; //flaga informujaca czy ciezarowka jest przy rampie

volatile sig_atomic_t wznowiono = 0;
semafor* global_sem = nullptr;
bool zatrzymany_z_rampa = false;

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
		wymuszony_odjazd = true; //flaga przerywajaca petle ladowania
	}
}

//funkcja obslugujaca wstrzymanie i kontynuacje pracy procesow
void obsluga_sigcont(int sig)
{
	if (sig == SIGCONT)
	{
		if (przy_rampie && global_sem != nullptr)
		{
			//zwolnij wszystko i zakoncz proces
			global_sem->v(3);
			loguj(CIEZAROWKA, "SIGCONT: Zwalniam zasoby i koncze proces\n");
			_exit(0); //zakoncz proces
		}
	}
}

//funkcja obslugujaca wstrzymanie i kontynuacje proecsow przyciskiem
void obsluga_pause(int sig)
{
	if (sig == SIGUSR2)
	{
		semafor sem(5, false);
		shared_memory pam(false);

		sem.p(0);
		pam.dane()->system_paused = !pam.dane()->system_paused;
		bool paused = pam.dane()->system_paused;
		sem.v(0);

		if (paused)
		{
			loguj(SYSTEM, "\n SYMULACJA WSTRZYMANA (nacisnij 'p' aby wznowic) \n");
		}
		else
		{
			loguj(SYSTEM, "\n SYMULACJA WZNOWIONA \n");
		}
	}
}


int main(int argc, char* argv[])
{
	//inicjalizacja zasobow systemowych
	semafor sem(5);
	shared_memory pam;
	kolejka kol;
	global_sem = &sem;

	//rejestracja sygnalow
	signal(SIGTERM, pracowanie);
	signal(SIGINT, pracowanie);
	signal(SIGUSR1, ekspresowy_odjazd);
	//signal(SIGCONT, obsluga_sigcont);
	signal(SIGUSR2, obsluga_pause);

	stan_tasmy* st = pam.dane(); //wskaznik na strukture w pamieci wspoldzielonej

	//glowna petla pracy ciezarowki, dziala dopoki jest flaga lub na tasmie sa paczki
	while (czy_pracowac || st->aktualna_liczba_paczek > 0) 
	{

		if (wznowiono)
		{
			loguj(CIEZAROWKA, "Ciezarowka wznowiona, podjezdam pod rampe\n");
			wznowiono = 0;
			//flaga zatrzymany_z_rampa juz jest na false
		}

		//podjazd pod rampe
		sem.p(3); //opuszczenie semafora (semafor nr. 3 kontroluje czy jest ciezarowka przy rampie)
		zatrzymany_z_rampa = true;
		przy_rampie = true;
		if (czy_pracowac) //ciezarowka podjezdza pod rampe tylko gdy magazyn pracuje
		{
			loguj(CIEZAROWKA, "Ciezarowka przy rampie\n");
		}
		//resetowanie zmiennych dla nowej ciezaorwki
		bool pelna = false;
		wymuszony_odjazd = false;
		double waga_ciezarowki = 0.0;
		double objetosc_ciezarowki = 0.0;

		//petla ladowania, dziala dopoki jest miejsce i nie ma rozkazu odjazdu
		while (!pelna) 
		{
			struct komunikat msg;

			while (true) //petla sprawdzajaca pauze
			{
				sem.p(0);
				bool paused = st->system_paused;
				sem.v(0);

				if (!paused) break; //brak pauzy - kontynuuj

				//sched_yield(); //oddaj cpu
			}

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
			while (!pelna && kol.odbierz_nieblokujaco(4, msg) != -1) 
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
					break; //przerwanie petli
				}
				if (pelna)//jesli ekspresy zapelnily ciezarowke petla ladowania ekspresowych zostjae przerwana
				{
					break;
				}

			}
			if (pelna)//jesli ekspresy zapelnily ciezarowke petla ladowania wszystkich paczek zostaje przerwana
			{
				break;
			}

			//obsluga paczek standardowo - oczekiwanie z tasmy
			bool paczka_odebrana = false;
			while (!paczka_odebrana && !wymuszony_odjazd)
			{
				if (sem.p_nieblokujace(2)) //proboj odebrac paczke nieblokujaco
				{
					paczka_odebrana = true;
				}
				//petla spraedzajaca czy jest wymuszony odjazd
			}

			if (wymuszony_odjazd)
			{
				loguj(CIEZAROWKA, "CIEZAROWKA: Otrzymalem rozkaz natychmiastowego odjazdu, sprawdzam czy sa ekspresy\n");
				struct komunikat msg_ekspres;
				while (!pelna && kol.odbierz_nieblokujaco(4, msg_ekspres) != -1)
				{
					double waga_ekspresowych = atof(msg_ekspres.text);
					double objetosc_ekspresowych = 0.2;

					if (waga_ciezarowki + waga_ekspresowych <= MAX_WAGA_CIEZAROWKA && objetosc_ciezarowki + objetosc_ekspresowych <= MAX_OBJETOSC_CIEZAROWKA)
					{
						waga_ciezarowki += waga_ekspresowych;
						objetosc_ciezarowki += objetosc_ekspresowych;
						loguj(CIEZAROWKA, "CIEZAROWKA: Zaladowalem ostatni ekspres %.2f przed odjazdem - laczna waga: %.2f\n", waga_ekspresowych, waga_ciezarowki);
					}
					else
					{
						kol.wyslij(4, msg_ekspres.id_nadawcy, msg_ekspres.text);
						loguj(CIEZAROWKA, "CIEZAROWKA: Ekspres %.2f sie nie zmiescil\n", waga_ekspresowych);
						break;
					}
				}
				break; //wymuszony odjazd - wychodzenie z petli ladowania
			}

			if (!paczka_odebrana) //nie ma paczki
			{
				continue; // spróbuj ponownie
			}


			//sprawdzenie czy podczas czekania na zwykla paczke nie pojawil sie pakiet ekspresowy
			if (kol.odbierz_nieblokujaco(4, msg) != -1)
			{
				sem.v(2); //oddanie semafora (odrzucenie zwyklej paczki)
				kol.wyslij(4, msg.id_nadawcy, msg.text); //zwrot wiadomosci z paczka do kolejki aby nie przepadla
				continue; //powrot do petli ekspresowych paczek
			}

			if (wymuszony_odjazd)
			{
				sem.v(2); //oddanie semafora bo jednak nie bierzemy paczki
				loguj(CIEZAROWKA, "CIEZAROWKA: Otrzymalem rozkaz natychmiastowego odjazdu przed pobraniem paczki\n");
				break; //natychmiastowy odjazd
			}

			sem.p(0); //mutex pamieci

			if (wymuszony_odjazd)
			{
				sem.v(0); //odblokuj mutex
				sem.v(2); //oddaj semafor paczki
				loguj(CIEZAROWKA, "CIEZAROWKA: Otrzymalem rozkaz natychmiastowego odjazdu w sekcji krytycznej\n");
				break;
			}

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

				struct komunikat msg_check;
				if (kol.odbierz_nieblokujaco(4, msg_check) != -1)
				{
					//jest pakiet ekspresowy - powrot do petli aby go zaladowac
					kol.wyslij(4, msg_check.id_nadawcy, msg_check.text); // oddaj do kolejki
					loguj(CIEZAROWKA, "CIEZAROWKA: Wykrylem pakiet ekspresowy, przerywam ladowanie zwyklych paczek\n");
					continue; //wrocenie na poczatek petli while
				}

				if (wymuszony_odjazd)
				{
					loguj(CIEZAROWKA, "CIEZAROWKA: Otrzymalem rozkaz odjazdu po zaladowaniu paczki\n");
					break; //natychmiastowy odjazd
				}
			}
			//brak miejsca
			else
			{
				pelna = true;
				sem.v(0); //odblokowanie pamieci
				sem.v(2); //oddanie semafora, paczka zostanie na tasmie dla nastepnej ciezarowki
			}
		}
		//odjazd ciezarowki (w logach nie pokazuja sie ciezaorwki ktore nie zaladowaly nic)
		if (waga_ciezarowki > 0.01)
		{
			loguj(CIEZAROWKA, "CIEZARKOWA: Odjezdzam z waga %.2f\n", waga_ciezarowki);
		}
		przy_rampie = false;
		zatrzymany_z_rampa = false;
		sem.v(3); //zwolnienie rampy
		//symulacja podrozy ciezarowki - zabezpieczona tak zeby nie przerywal podrozy sygnal
		/*
		if (czy_pracowac)
		{
			loguj(INFO, "CIEZAROWKA: wyruszam w trase na %d s\n", CZAS_JAZDY);

			unsigned int czas_do_spania = CZAS_JAZDY;

			while (czas_do_spania > 0 && czy_pracowac)
			{
				czas_do_spania = sleep(czas_do_spania);

			}

			if (czy_pracowac)
			{
				loguj(INFO, "Ciezarowka wrocila z trasy\n");
			}
		}
		*/
			
	}
	return 0;
}



	
