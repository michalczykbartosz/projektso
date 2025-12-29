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
#include <csignal>

bool system_dziala = true;

void obsluga_konca(int sig)
{
    system_dziala = false;
}

int main()
{
    setbuf(stdout, NULL);
    semafor sem(4);
    shared_memory pamiec;
    kolejka kol;

    signal(SIGINT, obsluga_konca);
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

    if (fork() == 0)
    {
        signal(SIGINT, SIG_IGN);
        while (true)
        {
            char c = getchar();

            if (c == '1')
            {
                system("pkill -SIGUSR1 ciezarowka");
            }
            else if (c == '2')
            {
                system("pkill -f 'pracownik 4' -SIGUSR2");
            }
            else if (c == '3')
            {
                kill(getppid(), SIGINT);
                exit(0);
            }
        }
        exit(0);
    }

    while (system_dziala)
    {
        komunikat msg = kol.odbierz(0);
        if (!system_dziala) break;
        printf("%s\n", msg.text);
    }
    printf("Rozpoczynam procedure konczenia pracy magazynu\n");
    system("pkill -SIGINT pracownik");
    for (int i = 0; i < 5; i++) sem.v(1);
    sleep(1);
    system("pkill -SIGINT ciezarowka");
    sem.v(2);
    sem.v(2);
    sem.v(0);
    while (wait(NULL) > 0);
    printf("Magazyn zakonczyl prace");
    return 0;
}