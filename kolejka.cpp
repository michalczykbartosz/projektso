#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cerrno>
#include "kolejka.h"
#include "semafory.h"

kolejka::kolejka(bool wlasciciel) //konstruktor - tworzenie klucza do kolejki komunikatow
{
	czy_wlasciciel = wlasciciel;
	key_t klucz;// = 33333;
	klucz = ftok(".", 'K');

	if (klucz == -1)
	{
		perror("Blad ftok() przy kolejce komunikatow");
		exit(1);
	}


	id_kolejka = msgget(klucz, IPC_CREAT | 0600); //tworzenie kolejki

	//wyjscie jesli kolejka sie nie utworzyla
	if (id_kolejka == -1)
	{
		perror("Blad przy tworzeniu kolejki komunikatow!"); //wypisanie bledu 
		exit(1);
	}

}

kolejka::~kolejka() //destruktor ktory automatycznie usuwa kolejke komunikatow jesli jest wywolywany przez wlasciciela
{
	if (czy_wlasciciel)
	{
		if (msgctl(id_kolejka, IPC_RMID, nullptr) == -1)
		{
			perror("Blad usuwania kolejki komunikatow");
		}
		else
		{
			printf("Kolejka komunikatow usunieta poprawnie\n");
		}
	}
}

//funkcja dodajaca komunikat do kolejki 
void kolejka::wyslij(int typ,int id_nadawcy, const char* tekst) 
{
	struct komunikat msg;

	msg.mtype = typ; //ustawianie typu wiadomosci
	msg.id_nadawcy = id_nadawcy; //ustawianie id_nadawcy

	strncpy(msg.text, tekst, sizeof(msg.text) - 1); //kopiowanie tekstu do wiadomosci kolejki
	msg.text[sizeof(msg.text) - 1] = '\0'; //upewnienie sie ze na koncu wiadomosci jest "\0"

	semafor sem(5); //podlaczenie sie do zbioru semaforow
	sem.p(4); //opuszczenie semaforu (czekanie jesli kolejka jest pelna)

	//zabezpieczenie przepelnienia kolejki, typ 1 (logi) sa w trybie nieblokujacym, typ 4 (ekspresowe) w blokujacym
	int flags;
	if (typ == 1)
	{
		flags = IPC_NOWAIT; //logi - nie czekaj gdy kolejka jest pelna
	}
	else
	{
		flags = 0; //ekspresy - czekaj na wolne miejsce
	}

	//jesli nie udalo sie wyslac wiadomosci, wypisujemy blad (rozmiar to wielkosc struktury - pole mtype)
	if (msgsnd(id_kolejka,&msg, sizeof(komunikat) - sizeof(long int), flags) == -1)
	{
		perror("Blad wysylania komunikatu!"); //wypisanie bledu
		sem.v(4);//oddanie semaforu po bledize
		return;
	}
}

//funkcja odbierajaca wiadomosc z kolejki
komunikat kolejka::odbierz(int typ) 
{
	komunikat msg;
	memset(&msg, 0, sizeof(msg)); //zerowanie pamieci przed odebraniem

	//jesli nie udalo sie odebrac wiadomosci, wypisujemy blad
	if (msgrcv(id_kolejka, &msg, sizeof(msg) - sizeof(long), typ, 0) == -1)
	{
		if (errno != EINTR) //wypisz tylko jesli nie jest to przerwanie sygnalem
		{
			perror("Blad odbierania komunikatu");
		}
	}

	semafor sem(5);
	sem.v(4);
	return msg;
}
//funkcja odbierania komunikatu z kolejki do sprawdzania czy jest pakiet ekspresowy, tak aby nie blokowalo pracy ciezarowki
int kolejka::odbierz_nieblokujaco(int typ, komunikat& msg) 

{
	int wynik=  msgrcv(id_kolejka, &msg, sizeof(msg) - sizeof(long), typ, IPC_NOWAIT);

	if (wynik != -1)
	{
		semafor sem(5);
		sem.v(4);
	}
	return wynik;
}










