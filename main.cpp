#include <cstdio>
#include <cstdlib>      
#include <unistd.h>
#include <sys/wait.h>
#include "semafory.h"
#include "pamiec_wspoldzielona.h"
#include "kolejka.h"    
#include "wspolne.h"
#include "bledy.h"
#include <cstring>

int main()
{
    semafor sem(4);
    shared_memory pamiec;
    kolejka kol;

    stan_tasmy* s = pamiec.dane();
    memset(s, 0, sizeof(stan_tasmy));
    s->dziala = true;

    pamiec.dane()->head = 0;
    pamiec.dane()->tail = 0;
    pamiec.dane()->aktualna_liczba_paczek = 0;
    pamiec.dane()->aktualna_waga_paczek_tasma = 0.0;
    pamiec.dane()->dziala = true;

    sem.ustaw(0, 1);            //mutex
    sem.ustaw(1, MAX_PACZEK);   //wolne
    sem.ustaw(2, 0);            //zajete
    sem.ustaw(3, 1);            //rampa

    for (int i = 1; i <= 4; i++)
    {
        if (fork() == 0)
        {
            char id_str[10];
            sprintf(id_str, "%d", i);
            execl("./pracownik", "pracownik", id_str, NULL);
            exit(1);
        }
    }

    for (int i = 0; i < LICZBA_CIEZAROWEK; i++)
    {
        if (fork() == 0)
        {
            execl("./ciezarowka", "ciezarowka", NULL);
            exit(1);
        }
    }

    while (true)
    {
        komunikat msg = kol.odbierz(0);
        printf("%s\n", msg.text);
    }

    return 0;
}