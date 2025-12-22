#include <cstdio>
#include <cstdlib>      
#include <unistd.h>
#include <sys/wait.h>
#include "semafory.h"
#include "pamiec_wspoldzielona.h"
#include "kolejka.h"    
#include "wspolne.h"
#include "bledy.h"

int main()
{
    semafor sem(3);
    shared_memory pamiec;
    kolejka kol;

    pamiec.dane()->head = 0;
    pamiec.dane()->tail = 0;
    pamiec.dane()->aktualna_liczba_paczek = 0;
    pamiec.dane()->aktualna_waga_paczek_tasma = 0.0;
    pamiec.dane()->dziala = true;

    sem.ustaw(0, 1);
    sem.ustaw(1, MAX_PACZEK);
    sem.ustaw(2, 0);

    printf("Uruchamiam pracownikow P1, P2, P3");

    for (int i = 1; i <= 3; i++)
    {
        if (fork() == 0) 
        {
            char id_str[10];
            sprintf(id_str, "%d", i); 

            execl("./pracownik", "pracownik", id_str, NULL);

            perror("FATAL ERROR: Nie widze pliku 'pracownik'! Czy go skompilowales?");
            exit(1);
        }
    }


    while (true)
    {
        komunikat msg = kol.odbierz(0);
        printf("[RAPORT] Od P%d: %s\n", msg.id_nadawcy, msg.text);
    }

    return 0;
}