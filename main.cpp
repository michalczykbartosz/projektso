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
#include "logger.h"

bool system_dziala = true; //flaga sterujaca symulacja

//funkcja zbierajaca procesy zombie
void obsluga_sigchld(int sig)
{
    while (waitpid(-1, NULL, WNOHANG) > 0); //WNOHANG - nie oczekuj tylko sprawdz i wroc
}

//funkcja ktora sygnalizuje koniec pracy po przyjeciu sygnalu SIGINT
void obsluga_konca(int sig) 
{
    system_dziala = false;
}

int main(int argc, char* argv[])
{
    int liczba_ciezarowek = LICZBA_CIEZAROWEK; //domyslna wartosc ze wspolne.h

    //obsluga argumentu wywolania
    if (argc > 1)
    {
        liczba_ciezarowek = atoi(argv[1]);
        if (liczba_ciezarowek < 1 || liczba_ciezarowek >10) //sprawdzdenie poprawnosci danych
        {
            bledy::rzuc_blad(3); //wypisanie bledu
        }
    }

    if (MAX_PACZEK <= 0) //walidacja MAX_PACZEK
    {
        bledy::rzuc_blad(5);
    }

    if (UDZWIG <= 0 || MAX_WAGA_CIEZAROWKA <= 0 || MAX_OBJETOSC_CIEZAROWKA <= 0 || CZAS_JAZDY <= 0) //walidacja stalych z wspolne.h
    {
        bledy::rzuc_blad(6);
    }

    setbuf(stdout, NULL); //wylaczenie bufora na stdout tak zeby nie czekalo na uzbieranie bloku tekstu
    //inicjalizacja zasobow systemowych
    semafor sem(4,true);
    shared_memory pamiec(true);
    kolejka kol(true);

    signal(SIGINT, obsluga_konca); //rejestracja sygnalu SIGINT do zakonczenia symulacji
    signal(SIGCHLD, obsluga_sigchld); //rejestracja sygnalu SIGCHLD do asynchronicznego zbierania procesow zombie
    stan_tasmy* s = pamiec.dane();
    memset(s, 0, sizeof(stan_tasmy));
    s->dziala = true;

    //inicjalizacja poczatkowych zmiennych symulacji
    pamiec.dane()->head = 0;
    pamiec.dane()->tail = 0;
    pamiec.dane()->aktualna_liczba_paczek = 0;
    pamiec.dane()->aktualna_waga_paczek_tasma = 0.0;
    pamiec.dane()->dziala = true;

    //ustawienie poczatkowych wartosci semaforow
    sem.ustaw(0, 1);            //mutex pamieci
    sem.ustaw(1, MAX_PACZEK);   //wolna tasma
    sem.ustaw(2, 0);            //zajeta tasma (paczka)
    sem.ustaw(3, 1);            //rampa

    //petla tworzaca pracownikow
    for (int i = 1; i <= 4; i++)
    {
        if (fork() == 0)
        {
            char id_str[10]; //bufor nazwy do logow
            sprintf(id_str, "%d", i); //wypisanie nazwy pracownika
            execl("./pracownik", "pracownik", id_str, NULL); //wykonanie programu pracownik
            _exit(1); //bezpieczne wyjscie gdy execl nie zadziala
        }
    }

    //petla tworzaca ciezarowki
    for (int i = 0; i < liczba_ciezarowek; i++) 
    {
        if (fork() == 0)
        {
            execl("./ciezarowka", "ciezarowka", NULL); //wykonanie programu ciezarowka
            _exit(1); //bezpieczne wyjscie gdy execl nie zadziala
        }
    }

    int pid_klawiatury = fork(); //utworzenie procesu do obslugi klawiatury i zapisanie jego pidu
    if (pid_klawiatury == 0)
    {
        while (true) //petla odczytujaca wejscie z klawiatury
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
                _exit(0);
            }
        }
        _exit(0);
    }

    //petla czekajaca na wiadomosci z kolejki komunikatow
    while (system_dziala)
    {
        komunikat msg = kol.odbierz(1);
        if (!system_dziala) break; //przerwanie petli gdy symulacja sie konczy
        loguj(INFO,"%s\n", msg.text); //koncowy log
    }

    loguj(SYSTEM, "Koncze prace magazynu\n");
    //zabezpieczenie dostepu do pamieci
    sem.p(0);
    s->dziala = false;
    sem.v(0);
    //lagodne zakonczenie procesow
    loguj(SYSTEM, "Wysylam SIGTERM do procesow\n");
    kill(pid_klawiatury, SIGTERM);
    system("pkill -TERM pracownik");
    system("pkill -TERM ciezarowka");
    loguj(SYSTEM, "Czekam 3 sekundy na zakonczenie procesow\n");
    sleep(3);
    //wymuszenie zakonczenia procesow ktore sie nie zakonczyly
    loguj(SYSTEM, "Wymuszam zakonczenie procesow ktorych nie udalo sie zamknac\n");
    kill(pid_klawiatury, SIGKILL);
    system("pkill -9 pracownik");
    system("pkill -9 ciezarowka");
    //koncowe sprzatanie procesow "zombie"
    loguj(SYSTEM, "Koncze pozostale procesy zombie\n");
    while (waitpid(-1, NULL, WNOHANG) > 0);
    loguj(SYSTEM, "Wszystkie procesy potomne zakonczone. Zwalnianie zasobow\n");





    /*
    //nowa poprawna logika usuwania
    loguj(SYSTEM, "Konzce prace magazynu\n");
    s->dziala = false; //zmiana stanu flagi dziala
    //zabijanie procesow
    kill(pid_klawiatury, SIGKILL);
    system("pkill pracownik");
    system("pkill ciezarowka");
    sleep(1);
    system("pkill -9 pracownik");
    system("pkill -9 ciezarowka");
    while (wait(NULL) > 0); //petla oczekujaca na zakonczenie procesow potomnych
    loguj(SYSTEM, "Procesy potomne zakonczone. Zwalnianie zasobow.\n");
    */
  
    return 0;

}