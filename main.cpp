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
#include <vector>

bool system_dziala = true; //flaga sterujaca symulacja
pid_t pid_klawiatury = -1; //pid procesu klawiatury

//funkcja zbierajaca procesy zombie
void obsluga_sigchld(int sig)
{
    while (waitpid(-1, NULL, WNOHANG) > 0); //WNOHANG - nie oczekuj tylko sprawdz i wroc
}

//funkcja ktora sygnalizuje koniec pracy po przyjeciu sygnalu SIGINT
void obsluga_konca(int sig) 
{
    system_dziala = false;
    if (pid_klawiatury > 0)//zabicie procesu klawiatury aby nie blokowal sie getchar()
    {
        kill(pid_klawiatury, SIGKILL);
    }
}

//funkcja obslugujaca pauze przyciskiem
void obsluga_pause(int sig)
{
    if (sig == SIGUSR2)
    {
        semafor sem(5, false);
        shared_memory pam(false);

        sem.p(0); 
        pam.dane()->system_paused = !pam.dane()->system_paused;
        bool paused = pam.dane()->system_paused;
        sem.v(0); 

        if (paused)
        {
            loguj(SYSTEM, "\n SYMULACJA WSTRZYMANA (nacisnij 'p' aby wznowic) \n");
        }
        else
        {
            loguj(SYSTEM, "\n SYMULACJA WZNOWIONA \n");
        }
    }
}



int main(int argc, char* argv[])
{
    int liczba_ciezarowek = LICZBA_CIEZAROWEK; //domyslna wartosc ze wspolne.h

    //obsluga argumentu wywolania
    if (argc > 1)
    {
        liczba_ciezarowek = atoi(argv[1]);
        if (liczba_ciezarowek < 1 || liczba_ciezarowek >20000) //sprawdzdenie poprawnosci danych
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
    semafor sem(5,true);
    shared_memory pamiec(true);
    kolejka kol(true);

    signal(SIGINT, obsluga_konca); //rejestracja sygnalu SIGINT do zakonczenia symulacji
    signal(SIGCHLD, obsluga_sigchld); //rejestracja sygnalu SIGCHLD do asynchronicznego zbierania procesow zombie
    signal(SIGUSR2, obsluga_pause);
    stan_tasmy* s = pamiec.dane();
    memset(s, 0, sizeof(stan_tasmy));

    //inicjalizacja poczatkowych zmiennych symulacji
    pamiec.dane()->head = 0;
    pamiec.dane()->tail = 0;
    pamiec.dane()->aktualna_liczba_paczek = 0;
    pamiec.dane()->aktualna_waga_paczek_tasma = 0.0;
    pamiec.dane()->dziala = true;
    pamiec.dane()->system_paused = false;

    //ustawienie poczatkowych wartosci semaforow
    sem.ustaw(0, 1);            //mutex pamieci
    sem.ustaw(1, MAX_PACZEK);   //wolna tasma
    sem.ustaw(2, 0);            //zajeta tasma (paczka)
    sem.ustaw(3, 1);            //rampa
    sem.ustaw(4, 150);          //miejsca w kolejce komunikatow

    //wektory przechowujace pidy wszystkich procesow potomnych
    std::vector<pid_t> pidy_pracownikow;
    pid_t pid_p4 = -1; //pid pracownika p4
    pid_t pgid_ciezarowek = 0; //pid grupy ciezarowek

    //petla tworzaca pracownikow
    for (int i = 1; i <= 4; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            char id_str[10]; //bufor nazwy do logow
            sprintf(id_str, "%d", i);//wypisanie nazwy pracownika
            execl("./pracownik", "pracownik", id_str, NULL); //wykonanie programu pracownik
            _exit(1); //bezpieczne wyjscie gdy execl nie zadziala
        }
        else if (pid > 0)
        {
            pidy_pracownikow.push_back(pid); //zapisanie w wektorze pidu pracownika
            if (i == 4)
            {
                pid_p4 = pid; //zapisanie pidu pracownika ekspresowego
                nice(0); //nadanie wyzszego priorytetu dla pracownika ekspresowego
            }
        }
    }

    printf("Uruchamianie pierwszej ciezarowki\n");
    pid_t pid_pierwszej = fork(); 

    if (pid_pierwszej == 0)
    {
        setpgid(0, 0); //zalozenie grupy procesow
        nice(19); //minimalny priorytet procesu
        execl("./ciezarowka", "ciezarowka", NULL);
        _exit(1);
    }
    else if (pid_pierwszej > 0)
    {
        pgid_ciezarowek = pid_pierwszej;
        setpgid(pid_pierwszej, pgid_ciezarowek);
    }
    else
    {
        perror("Blad fork() dla pierwszej cieazarowki");
        return 1;
    }

    pid_klawiatury = fork(); //uruchomienie osobnego procesu dla klawiatury

    if (pid_klawiatury == 0)
    {
        while (true) //petla odczytujaca wejscie z klawiatury
        {
            int x = getchar();
            if (x == '\n') continue; //ignorowanie znaku entera
            if (x == EOF) break; //zabezpiecznei przed bledem strumienia

            if (x == '1')
            {
                if (pgid_ciezarowek > 0)
                {
                    kill(-pgid_ciezarowek, SIGUSR1);
                    loguj(INFO, "Wyslano sygnal SIGUSR do grupy %d\n", pgid_ciezarowek);
                }
            }
            else if (x == '2')
            {
                if (pid_p4 > 0)
                {
                    kill(pid_p4, SIGUSR2);
                    loguj(SYSTEM, "Wysłano SIGUSR2 do pracownika P4\n");
                }
            }
            else if (x == '3')
            {
                kill(getppid(), SIGINT);
                _exit(0);
            }
            else if (x == 'p' || x == 'P')
            {
                kill(getppid(), SIGUSR2); 
                loguj(SYSTEM, "Przelaczono pause/resume\n");
            }
        }
        _exit(0);
    }

    //petla tworzaca ciezarowki
    if (liczba_ciezarowek > 1)
    {
        for (int i = 1; i < liczba_ciezarowek; i++)
        {
            pid_t pid = fork();

            if (pid < 0)
            {
                fprintf(stderr, "Limit procesow osiagniety przy ciezarowce nr %d\n", i);
                break;
            }
            if (pid == 0)
            {
                setpgid(0, pgid_ciezarowek); //przypisanie do grupy procesow
                nice(19); //minimalny priorytet
                execl("./ciezarowka", "ciezarowka", NULL);
                _exit(1);
            }
            else if (pid > 0)
            {
                setpgid(pid, pgid_ciezarowek); //zabezpieczajace przypisanie do grupy procesow
            }
        }
    }



    //petla czekajaca na wiadomosci z kolejki komunikatow
    while (system_dziala)
    {
        komunikat msg = kol.odbierz(1);
        if (!system_dziala) break; //przerwanie petli gdy symulacja sie konczy
        loguj(INFO,"%s\n", msg.text); //koncowy log
    }

    loguj(SYSTEM, "Koncze prace magazynu\n");
    if (pid_klawiatury > 0) //zabicie najpierw procesu klawiatury aby getchar() sie nie zwiesil
    {
        kill(pid_klawiatury, SIGKILL);
        waitpid(pid_klawiatury, NULL, 0); //czekanie na zakonczenie
    }   
    //zabezpieczenie dostepu do pamieci i ustawienie flagi ze magazyn zakonczyl prace
    sem.p(0);
    s->dziala = false;
    sem.v(0);
    //lagodne zakonczenie procesow
    loguj(SYSTEM, "Wysylam SIGTERM do procesow\n");
    for (size_t i = 0; i < pidy_pracownikow.size(); i++)
    {
        kill(pidy_pracownikow[i], SIGTERM); //zabijanie wszystkich pracownikow z wektora
    }
    if (pgid_ciezarowek > 0)
    {
        kill(-pgid_ciezarowek, SIGTERM);//zabijamy cala grupe ciezarowek
    }
    loguj(SYSTEM, "Czekam 3 sekundy na zakonczenie procesow\n");
    sleep(3);
    //wymuszenie zakonczenia procesow ktore sie nie zakonczyly
    loguj(SYSTEM, "Wymuszam zakonczenie procesow ktorych nie udalo sie zamknac\n");
    for (size_t i = 0; i < pidy_pracownikow.size(); i++)
    {
        kill(pidy_pracownikow[i], SIGKILL);
    }

    if (pgid_ciezarowek > 0)
    {
        kill(-pgid_ciezarowek, SIGKILL);
    }
    //koncowe sprzatanie procesow "zombie"
    loguj(SYSTEM, "Koncze pozostale procesy zombie\n");
    while (waitpid(-1, NULL, WNOHANG) > 0);
    loguj(SYSTEM, "Wszystkie procesy potomne zakonczone. Zwalnianie zasobow\n");

  
    return 0;

}