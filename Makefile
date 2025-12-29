all: main pracownik ciezarowka


main: main.cpp semafory.cpp pamiec_wspoldzielona.cpp kolejka.cpp bledy.cpp

	g++ -o main main.cpp semafory.cpp pamiec_wspoldzielona.cpp kolejka.cpp bledy.cpp


pracownik: pracownik.cpp semafory.cpp pamiec_wspoldzielona.cpp kolejka.cpp bledy.cpp

	g++ -o pracownik pracownik.cpp semafory.cpp pamiec_wspoldzielona.cpp kolejka.cpp bledy.cpp


ciezarowka: ciezarowka.cpp semafory.cpp pamiec_wspoldzielona.cpp kolejka.cpp bledy.cpp

	g++ -o ciezarowka ciezarowka.cpp semafory.cpp pamiec_wspoldzielona.cpp kolejka.cpp bledy.cpp


run: all

	-ipcrm -a

	stdbuf -o0 ./main | tee raport.txt


clean:

	rm -f main pracownik ciezarowka raport.txt

	-ipcrm -a

