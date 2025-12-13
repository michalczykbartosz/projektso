#pragma once

#define MAX_PACZEK 50
#define MAX_WAGA 500

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