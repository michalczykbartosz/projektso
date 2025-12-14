#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include "semafory.h"

semafor::semafor(int ilosc)
{
	key_t klucz;
	pid_dyspozytora = getpid();

	klucz = ftok(".", 'B'); //tworzenie klucza do zbioru semaforow

	id_semafor = semget(klucz, ilosc, IPC_CREAT | 0666); //tworzenie zbioru semaforow 

	if (getpid() == pid_dyspozytora)
	{
		for (int i = 0; i < ilosc; i++)
		{
			semctl(id_semafor, i, SETVAL,1); //ustawianie kazdego semafora w zbiorze na 1
		}
	}


}

semafor::~semafor()
{
	if (getpid() == pid_dyspozytora) //destruktor ktory usuwa zbior semaforow kiedy dyspozytor konczy prace
	{
		semctl(id_semafor, 0, IPC_RMID);
	}
}

void semafor::ustaw(int nr_semafor, int wartosc) //funkcja ktora ustawia semafor na dana wartosc
{
	if (getpid() == pid_dyspozytora)
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
	

	semop(id_semafor, &operacja, 1);
}

void semafor::p(int nr_semafor) //operacja zniesienia semafora o 1
{
	struct sembuf operacja;
	operacja.sem_num = nr_semafor;
	operacja.sem_op = -1;
	operacja.sem_flg = 0;


	semop(id_semafor, &operacja, 1);
}