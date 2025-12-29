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

	pid_dyspozytora = getpid();

	id_kolejka = msgget(klucz, IPC_CREAT | 0600);

	if (id_kolejka == -1)
	{
		perror("Blad przy tworzeniu kolejki komunikatow!");
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

void kolejka::wyslij(int typ,int id_nadawcy, const char* tekst) //funkcja dodajaca komuniakat do kolejki 
{
	struct komunikat msg;

	msg.mtype = typ;
	msg.id_nadawcy = id_nadawcy;

	strncpy(msg.text, tekst, sizeof(msg.text) - 1);
	msg.text[sizeof(msg.text) - 1] = '\0';



	if (msgsnd(id_kolejka,&msg, sizeof(komunikat) - sizeof(long int), 0) == -1)
	{
		perror("Blad wysylania komunikatu!");
	}
}

komunikat kolejka::odbierz(int typ) //funkcja pobierania komunikatu z kolejki
{
	komunikat msg;

	if (msgrcv(id_kolejka, &msg, sizeof(msg) - sizeof(long), typ, 0) == -1)
	{
		perror("Blad odbierania komunikatu!");
	}
	return msg;
}

int kolejka::odbierz_nieblokujaco(int typ, komunikat& msg) //funkcja do sprawdzania czy w ogole jest jakis komunikat (dla ciezarowki) i nie blokuje

{
	return msgrcv(id_kolejka, &msg, sizeof(msg) - sizeof(long), typ, IPC_NOWAIT);
}










