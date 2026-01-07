#pragma once
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

class semafor
{
private:
	int id_semafor; //id zbioru semaforow
	bool czy_wlasciciel; //flaga sterujaca usuwaniem semaforow w destruktorze

public:
	semafor(int ilosc, bool wlasciciel = false); //konstruktor tworzacy zbior semaforow

	~semafor(); //destruktor usuwajacy zbior semaforow jesli jest wywolywany przez wlasciciela

	void ustaw(int nr_semafor, int wartosc); //funkcja ustawiajaca wartosc semaforu

	void v(int nr_semafor); //funkcja podnoszaca semafor o 1

	void p(int nr_semafor); //funkcja opuszczajaca semafor o 1
};