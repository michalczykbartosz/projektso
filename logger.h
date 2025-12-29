#pragma once

enum LogType
{
	INFO,
	PRACOWNIK,
	P4,
	CIEZAROWKA,
	SYSTEM,
	BLAD
};

void loguj(LogType typ, const char* format, ...);