#pragma once

class bledy
{
private:
	static const char* komunikaty[];
	static int ilosc_bledow;
public:
	static void	rzuc_blad(int id_bledu);
};