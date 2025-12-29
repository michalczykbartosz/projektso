all: main pracownik ciezarowka


main: main.cpp semafory.cpp pamiec_wspoldzielona.cpp kolejka.cpp bledy.cpp logger.cpp

	g++ -o main main.cpp semafory.cpp pamiec_wspoldzielona.cpp kolejka.cpp bledy.cpp logger.cpp


pracownik: pracownik.cpp semafory.cpp pamiec_wspoldzielona.cpp kolejka.cpp bledy.cpp logger.cpp

	g++ -o pracownik pracownik.cpp semafory.cpp pamiec_wspoldzielona.cpp kolejka.cpp bledy.cpp logger.cpp


ciezarowka: ciezarowka.cpp semafory.cpp pamiec_wspoldzielona.cpp kolejka.cpp bledy.cpp logger.cpp

	g++ -o ciezarowka ciezarowka.cpp semafory.cpp pamiec_wspoldzielona.cpp kolejka.cpp bledy.cpp logger.cpp


run: all

	-ipcrm -a
	rm -f raport.txt
	./main


clean:

	rm -f main pracownik ciezarowka raport.txt

	-ipcrm -a

