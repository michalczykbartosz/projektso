#pragma once

#ifndef MAX_PACZEK
#define MAX_PACZEK 50
#endif
#ifndef UDZWIG
#define UDZWIG 500
#endif
#ifndef MAX_OBJETOSC_CIEZAROWKA
#define MAX_OBJETOSC_CIEZAROWKA 1
#endif
#ifndef MAX_WAGA_CIEZAROWKA
#define MAX_WAGA_CIEZAROWKA 100
#endif
#ifndef LICZBA_CIEZAROWEK
#define LICZBA_CIEZAROWEK 3
#endif
#ifndef CZAS_JAZDY
#define CZAS_JAZDY 15
#endif

struct Paczka
{
	int id_pracownika;
	char typ;
	double waga;
	bool priorytet;
};

struct stan_tasmy
{
	Paczka bufor[MAX_PACZEK];
	int head;
	int tail;
	int aktualna_liczba_paczek;
	double aktualna_waga_paczek_tasma;
	bool dziala;
	bool system_paused;
};

struct komunikat
{
	long int mtype;
	int id_nadawcy;
	char text[100];
};