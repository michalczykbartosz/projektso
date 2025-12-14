#include <cstdio>
#include <stdio.h>
#include "wspolne.h"
#include "semafory.h"
#include "pamiec_wspoldzielona.h"

int main()
{
    printf("hello from %s!\n", "projektso");
    
    semafor s(1);
    shared_memory pamiec;

    pamiec.dane()->head = 10;
    pamiec.dane()->dziala = true;

    s.p(0);
    printf("Zapisano: %d\n", pamiec.dane()->head);
    s.v(0);

    return 0;
}