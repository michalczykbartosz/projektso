#pragma once
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include "wspolne.h"


class shared_memory
{
private:
	int id_pamieci;
	stan_tasmy* adres;
	int pid_dyspozytora;

public:
	shared_memory();
	~shared_memory();
	stan_tasmy* dane();
};
