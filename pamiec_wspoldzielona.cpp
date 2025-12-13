#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <cstdio>
#include "wspolne.h"
#include "pamiec_wspoldzielona.h"

shared_memory::shared_memory()
{
	key_t klucz;
	klucz = ftok(".", 'B');

	id_pamieci = shmget(klucz, sizeof(stan_tasmy), IPC_CREAT | 0666);
	adres = (stan_tasmy*) shmat(id_pamieci, nullptr,0);
	pid_dyspozytora = getpid();
}

shared_memory::~shared_memory()
{
	shmdt(adres);

	if (getpid() == pid_dyspozytora)
	{
		shmctl(id_pamieci,IPC_RMID,nullptr);
	}

}

stan_tasmy* shared_memory::dane()
{
	return adres;
}