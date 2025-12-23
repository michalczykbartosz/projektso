#pragma once

#define MAX_PACZEK 50
#define UDZWIG 500
#define MAX_OBJETOSC_CIEZAROWKA 50.0
#define MAX_WAGA_CIEZAROWKA 100
#define LICZBA_CIEZAROWEK 3

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
};

struct komunikat
{
	long int mtype;
	int id_nadawcy;
	char text[100];
};