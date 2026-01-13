#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include "semafory.h"


/*
semafor 0 - dostep do pamieci
semafor 1 - wolne miejsca na tasmie
semafor 2 - gotowe paczki do odbioru
semafor 3 - rampa zeby tylko 1 ciezarowka byla
*/

//konstruktor tworzacy semafor 
semafor::semafor(int ilosc,bool wlasciciel)
{
	czy_wlasciciel = wlasciciel; //ustawienie flagi czy_wlasciciel przez argument
	key_t klucz;
	klucz = ftok(".", 'S'); //tworzenie klucza do zbioru semaforow

	if (klucz == -1) //wypisanie bledu jesli nie udalo sie stworzyc klucza
	{
		perror("Blad ftok() przy semaforach");
		exit(1);
	}

	id_semafor = semget(klucz, ilosc, IPC_CREAT | 0600); //tworzenie zbioru semaforow 

	if (id_semafor == -1) //wypisanie bledu jesli tworzenie zbioru semaforow sie nie powiedzie
	{
		perror("Blad przy tworzeniu semaforow!"); 
		exit(1);
	}

}


//destruktor usuwajacy zbior semaforow jesli zostnaie wywolany przez wlasciciela (flaga czy_wlasciciel)
semafor::~semafor()
{
	
	if (czy_wlasciciel) 
	{
		if (semctl(id_semafor, 0, IPC_RMID) == -1) //jesli usuwanie nie powiedzie sie, wypisywany jest blad
		{
			perror("Blad usuwania semaforow");
		}
		else
		{
			printf("Semafory poprawnie usuniete\n"); //wypisanie potwierdzenia usuniecia zbioru semaforow
		}
	}
	
}


void semafor::ustaw(int nr_semafor, int wartosc) //funkcja ustawiajaca dany semafor na wybrana wartosc
{
	if (czy_wlasciciel)
	{
		if (semctl(id_semafor, nr_semafor, SETVAL, wartosc) == -1) //wypisanie bledu jesli nie udalo sie ustawic wartosci semafora
		{
			perror("Blad ustawiania wartosci semafora");
		}
	}
}


void semafor::v(int nr_semafor) //operacja podniesienia semafora o 1
{
	struct sembuf operacja;
	//inicjalizacja zmiennych lokalnych
	operacja.sem_num = nr_semafor;
	operacja.sem_op = 1;
	operacja.sem_flg = 0;
	

	while (semop(id_semafor, &operacja, 1) == -1)
	{
		if (errno == EINTR) continue; //ponowienie proby jesli przerwano przez sygnal
		
			perror("Blad operacji V semafora"); //wypisanie bledu jesli operacja podniesienia sie nie powiedzie
			exit(1);
		
	}
}

void semafor::p(int nr_semafor) //operacja opuszczenia semafora o 1
{
	struct sembuf operacja;
	//inicjalizacja zmiennych lokalnych
	operacja.sem_num = nr_semafor;
	operacja.sem_op = -1;
	operacja.sem_flg = 0;


	while(semop(id_semafor, &operacja, 1) == -1) 
	{
		if (errno == EINTR) continue; //ponowienie proby jesli przerwano przez sygnal
		
			perror("Blad operacji P semafora"); //wypisanie bledu jesli operacja opuszczenia sie nie powiedzie
			exit(1);
		
	}
}

bool semafor::p_przerywalne(int nr_semafor)
{
	struct sembuf operacja;
	//inicjalizacja zmiennych lokalnych
	operacja.sem_num = nr_semafor;
	operacja.sem_op = -1;
	operacja.sem_flg = 0;

	if (semop(id_semafor, &operacja, 1) == -1)
	{
		if (errno == EINTR) //jezeli odebrano sygnal podczas opuszczania semafora zwracane jest false
		{
			return false;
		}
		perror("Blad operacji P semafora");
		exit(1);
	}
	return true;
}