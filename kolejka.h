#pragma once
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "wspolne.h"

class kolejka
{
private:
	int id_kolejka;
	int pid_dyspozytora;

public:
	kolejka();
	~kolejka();

	void wyslij(int id_nadawcy,const char* tekst);
	
	komunikat odbierz(int typ = 0);

	int odbierz_nieblokujaco(int typ, komunikat& msg);

	

};



