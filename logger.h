#pragma once

// typy logow uzywane do kolorowania w konsoli
enum LogType
{
	INFO, // zwykla informacja
	PRACOWNIK, //logi pracownikow
	P4, //logi pracownika ekspresowego
	CIEZAROWKA, //logi ciezarowek
	SYSTEM, //logi systemowe i dyspozytora
	BLAD //bledy
};


//glowna funkcja logujaca: typ - typ logu, format - tekst z formatowaniem, ... - lista argumentow do formatowania (jak w printf)
void loguj(LogType typ, const char* format, ...);