#include <cstdio>
#include <stdio.h>
#include "wspolne.h"
#include "pamiec_wspoldzielona.h"

int main()
{
    printf("hello from %s!\n", "projektso");
    
    shared_memory pamiec;

    pamiec.dane()->head = 10;
    pamiec.dane()->dziala = true;

    printf("Zapisano: %d\n", pamiec.dane()->head);

    return 0;
}