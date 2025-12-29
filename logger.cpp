#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <cstring>
#include "logger.h"

//predefiniowane zmienne do kolorowania tekstu w terminalu
#define RESET   "\033[0m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"

//funkcja wypisujaca logi w konsoli oraz zapisująca do pliku wraz z czasem i kolorem
void loguj(LogType typ, const char* format, ...)
{
	time_t now = time(0); //pobieranie czasu
	struct tm tstruct;
	char time_buf[80]; //bufor na dokladny czas
	tstruct = *localtime(&now); //konwertowanie czasu na strukture lokalna
	strftime(time_buf, sizeof(time_buf), "[%H:%M:%S]", &tstruct); //formatowanie czasu

	const char* kolor = RESET;

	switch (typ)
	{
	case INFO: kolor = RESET; break;
	case PRACOWNIK: kolor = GREEN; break;
	case P4: kolor = MAGENTA; break;
	case CIEZAROWKA: kolor = CYAN; break;
	case SYSTEM: kolor = YELLOW; break;
	case BLAD: kolor = RED; break;
	}

	va_list args; 
	va_start(args, format); //inicjalizacja listy argumentow
	printf("%s %s", kolor, time_buf);  //wypisywanie koloru i czasu
	vprintf(format, args); //wypisywanie tresci komunikatu
	printf("%s", RESET); //resetowanie koloru
	va_end(args); //resetowanie listy argumentow

	FILE* f = fopen("raport.txt", "a"); //otwarcie pliku w trybie "a"
	if (f)
	{
		//zapisywanie czasu i tresci do pliku
		va_start(args, format);
		fprintf(f, "%s ", time_buf);
		vfprintf(f, format, args);
		va_end(args);
		fclose(f);
	}

}