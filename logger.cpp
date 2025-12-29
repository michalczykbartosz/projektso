#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <cstring>
#include "logger.h"

#define RESET   "\033[0m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"

void loguj(LogType typ, const char* format, ...)
{
	time_t now = time(0);
	struct tm tstruct;
	char time_buf[80];
	tstruct = *localtime(&now);
	strftime(time_buf, sizeof(time_buf), "[%H:%M:%S]", &tstruct);

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
	va_start(args, format);
	printf("%s %s", kolor, time_buf);
	vprintf(format, args);
	printf("%s", RESET);
	va_end(args);

	FILE* f = fopen("raport.txt", "a");
	if (f)
	{
		va_start(args, format);
		fprintf(f, "%s ", time_buf);
		vfprintf(f, format, args);
		va_end(args);
		fclose(f);
	}

}