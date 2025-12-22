#include <cstdio>
#include <cstdlib>
#include "bledy.h"

const char* bledy::komunikaty[] = {
	"Wystapil nieznany blad!", //0
	"Liczba pracownikow musi byc wieksza niz 0!", //1
	"Przekroczono limit pracownikow!", //2
	"Liczba kierowcow musi byc wieksza niz 0!", //3
};

int bledy::ilosc_bledow = sizeof(bledy::komunikaty) / sizeof(bledy::komunikaty[0]);

void bledy::rzuc_blad(int id_bledu)
{
	if (id_bledu < 0 || id_bledu>=ilosc_bledow)
	{
		id_bledu = 0;
	}
	fprintf(stderr, "Wsytapil blad: %s\n", komunikaty[id_bledu]);
	exit(1);
}