#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include "kolejka.h"

kolejka::kolejka() //konstruktor - tworzenie klucza do kolejki komunikatow
{
	key_t klucz = 33333;
	//klucz = ftok("main.cpp", 'K');

	pid_dyspozytora = getpid(); //ustawienie pid_dyspozytora na proces tworzacy kolejke

	id_kolejka = msgget(klucz, IPC_CREAT | 0600); //tworzenie kolejki

	//wyjscie jesli kolejka sie nie utworzyla
	if (id_kolejka == -1)
	{
		perror("Blad przy tworzeniu kolejki komunikatow!"); //wypisanie bledu 
		exit(1);
	}

}

kolejka::~kolejka() //destruktor ktory automatycznie usuwa kolejke komunikatow jesli jest wywolywany przez dyspozytora
{
	if (getpid() == pid_dyspozytora)
	{
		//msgctl(id_kolejka, IPC_RMID, nullptr);
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


	//jesli nie udalo sie wyslac wiadomosci, wypisujemy blad (rozmiar to wielkosc struktury - pole mtype)
	if (msgsnd(id_kolejka,&msg, sizeof(komunikat) - sizeof(long int), 0) == -1)
	{
		perror("Blad wysylania komunikatu!"); //wypisanie bledu
	}
}

//funkcja odbierajaca wiadomosc z kolejki
komunikat kolejka::odbierz(int typ) 
{
	komunikat msg;

	//jesli nie udalo sie odebrac wiadomosci, wypisujemy blad
	if (msgrcv(id_kolejka, &msg, sizeof(msg) - sizeof(long), typ, 0) == -1)
	{
		perror("Blad odbierania komunikatu!"); //wypisanie bledu
	}
	return msg;
}
//funkcja odbierania komunikatu z kolejki do sprawdzania czy jest pakiet ekspresowy, tak aby nie blokowalo pracy ciezarowki
int kolejka::odbierz_nieblokujaco(int typ, komunikat& msg) 

{
	return msgrcv(id_kolejka, &msg, sizeof(msg) - sizeof(long), typ, IPC_NOWAIT);
}










