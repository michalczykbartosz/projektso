#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include "semafory.h"

semafor::semafor(int ilosc,bool wlasciciel)
{
	czy_wlasciciel = wlasciciel;
	key_t klucz;// = 22222;

	klucz = ftok(".", 'S'); //tworzenie klucza do zbioru semaforow

	id_semafor = semget(klucz, ilosc, IPC_CREAT | 0600); //tworzenie zbioru semaforow 

	if (id_semafor == -1)
	{
		perror("Blad przy tworzeniu semaforow!");
		exit(1);
	}

}


semafor::~semafor()
{
	
	if (czy_wlasciciel) //destruktor ktory usuwa zbior semaforow kiedy dyspozytor konczy prace
	{
		if (semctl(id_semafor, 0, IPC_RMID) == -1)
		{
			perror("Blad usuwania semaforow");
		}
		else
		{
			printf("Semafory poprawnie usuniete\n");
		}
	}
	
}


void semafor::ustaw(int nr_semafor, int wartosc) //funkcja ktora ustawia semafor na dana wartosc
{
	if (czy_wlasciciel)
	{
		semctl(id_semafor, nr_semafor, SETVAL, wartosc);
	}
}


void semafor::v(int nr_semafor) //operacja podniesienia semafora o 1
{
	struct sembuf operacja;
	operacja.sem_num = nr_semafor;
	operacja.sem_op = 1;
	operacja.sem_flg = 0;
	

	while (semop(id_semafor, &operacja, 1) == -1)
	{
		if (errno == EINTR) continue;
		
			perror("Blad operacji V semafora");
			exit(1);
		
	}
}

void semafor::p(int nr_semafor) //operacja zniesienia semafora o 1
{
	struct sembuf operacja;
	operacja.sem_num = nr_semafor;
	operacja.sem_op = -1;
	operacja.sem_flg = 0;


	while(semop(id_semafor, &operacja, 1) == -1)
	{
		if (errno == EINTR) continue;
		
			perror("Blad operacji P semafora");
			exit(1);
		
	}
}