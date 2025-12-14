#pragma once
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

class semafor
{
private:
	int id_semafor;
	int pid_dyspozytora;

public:
	semafor(int ilosc);

	~semafor();

	void ustaw(int nr_semafor, int wartosc);

	void v(int nr_semafor);

	void p(int nr_semafor);
};