MAX_PACZEK ?= 50
UDZWIG ?= 500
MAX_OBJETOSC_CIEZAROWKA ?= 1
MAX_WAGA_CIEZAROWKA ?= 100
LICZBA_CIEZAROWEK ?= 3
CZAS_JAZDY ?= 15

DEFINES = -DMAX_PACZEK=$(MAX_PACZEK) \
          -DUDZWIG=$(UDZWIG) \
          -DMAX_OBJETOSC_CIEZAROWKA=$(MAX_OBJETOSC_CIEZAROWKA) \
          -DMAX_WAGA_CIEZAROWKA=$(MAX_WAGA_CIEZAROWKA) \
          -DLICZBA_CIEZAROWEK=$(LICZBA_CIEZAROWEK) \
	  -DCZAS_JAZDY=$(CZAS_JAZDY)

FLAGS = $(DEFINES)

all: main pracownik ciezarowka

main: main.cpp semafory.cpp pamiec_wspoldzielona.cpp kolejka.cpp bledy.cpp logger.cpp
	g++ $(FLAGS) -o main main.cpp semafory.cpp pamiec_wspoldzielona.cpp kolejka.cpp bledy.cpp logger.cpp


pracownik: pracownik.cpp semafory.cpp pamiec_wspoldzielona.cpp kolejka.cpp bledy.cpp logger.cpp
	g++ $(FLAGS) -o pracownik pracownik.cpp semafory.cpp pamiec_wspoldzielona.cpp kolejka.cpp bledy.cpp logger.cpp


ciezarowka: ciezarowka.cpp semafory.cpp pamiec_wspoldzielona.cpp kolejka.cpp bledy.cpp logger.cpp
	g++ $(FLAGS) -o ciezarowka ciezarowka.cpp semafory.cpp pamiec_wspoldzielona.cpp kolejka.cpp bledy.cpp logger.cpp


run: clean all
	./main $(LICZBA_CIEZAROWEK)


clean:
	rm -f main pracownik ciezarowka raport.txt
	-ipcrm -a
