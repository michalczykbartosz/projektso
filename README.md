# Projekt Systemy Operacyjne - Temat 10: Magazyn Firmy Spedycyjnej.

**Imię:** Bartosz **Nazwisko:** Michalczyk **Nr. Albumu:** 156487 **Github:** https://github.com/michalczykbartosz/projektso

## 1. Szczegółowy opis projektu oraz zasady symulacji:
Projekt realizuje problem synchronizacji procesów w środowisku wielowątkowym/wieloprocesowym, modelując działanie magazynu spedycyjnego.
System składa się z niezależnych procesów symulujących pracowników, taśmę transportową oraz ciężarówki, zarządzanych przez proces dyspozytora.

### A. Procesy
1.  **Pracownicy Taśmowi (P1, P2, P3) – Producenci:**
    * Każdy pracownik obsługuje inny typ paczki (A, B, C).
    * Pracują w pętli nieskończonej, starając się jak najszybciej położyć paczkę na taśmie.
    * Przed położeniem paczki muszą sprawdzić **dwa warunki krytyczne** taśmy (ilość miejsc i udźwig).
2.  **Pracownik Ekspresowy (P4) – Producent Uprzywilejowany:**
    * Nie korzysta z taśmy transportowej.
    * Pozostaje w stanie uśpienia do momentu otrzymania **rozkazu (Sygnał 2)**.
    * Ma absolutny priorytet dostępu do ciężarówki (blokuje taśmę na czas swojego załadunku).
3.  **Ciężarówka – Konsument:**
    * Pobiera paczki z końca taśmy (zgodnie z kolejnością FIFO).
    * Sumuje wagę i objętość ładunku.
    * Odjeżdża po zapełnieniu lub na rozkaz dyspozytora.
4.  **Dyspozytor:**
    * Interfejs sterujący symulacją, wysyłający asynchroniczne sygnały do pozostałych procesów.

    ### B. Parametry i Ograniczenia (Zasady Matematyczne)

#### 1. Taśma Transportowa (Bufor Ograniczony)
Taśma jest zasobem dzielonym o ścisłych ograniczeniach. Wstawienie paczki o wadze $w$ jest możliwe wtedy i tylko wtedy, gdy spełnione są jednocześnie dwa warunki:
* $$Liczba_{paczek} < K$$ (Dostępne miejsce na taśmie)
* $$Suma_{wag} + w \le M$$ (Dostępny udźwig taśmy)

Jeśli którykolwiek warunek nie jest spełniony, pracownik (P1-P3) zostaje zablokowany na semaforze do momentu zwolnienia zasobów przez ciężarówkę.

#### 2. Definicje Paczek
Wagi paczek losowane są z zakresu **0.1 kg – 25.0 kg**. Przyjęto model, w którym waga koreluje z objętością (większa paczka ma większą szansę być cięższą).

| Pracownik | Typ | Wymiary [cm] | Objętość $V_p$ [$m^3$] | Uwagi |
| :--- | :---: | :--- | :--- | :--- |
| **P1** | A | 64x38x8 | 0.019 | Standardowa |
| **P2** | B | 64x38x19 | 0.046 | Standardowa |
| **P3** | C | 64x38x41 | 0.099 | Standardowa |
| **P4** | Mix | A/B/C | wg typu | Tylko paczki < 25kg |

#### 3. Logistyka Ciężarówek
W systemie krąży łącznie $N$ ciężarówek. W danej chwili przy rampie stoi tylko jedna.
Ciężarówka odjeżdża z magazynu (zwalnia miejsce dla nowej), gdy zajdzie jedno ze zdarzeń:
1.  **Przeładowanie wagowe:** $$Suma_{wag} + w_{nowa} > W$$
2.  **Przeładowanie objętościowe:** $$Suma_{obj} + V_{nowa} > V$$
3.  **Wymuszenie:** Otrzymano Sygnał 1 od dyspozytora.

Po odjeździe ciężarówka "rozwozi towar" przez czas $T_i$ (symulowany funkcją `sleep`), po czym wraca do kolejki oczekujących na wjazd.

### C. System Sygnałów (Sterowanie)
Symulacja obsługuje interwencje dyspozytora poprzez sygnały systemowe (mapowane na sygnały UNIX):

* **SYGNAŁ 1 (np. SIGUSR1): Wymuszony Odjazd**
    * Skierowany do: **Obecnej ciężarówki**.
    * Akcja: Ciężarówka natychmiast przerywa załadunek, zamyka drzwi i odjeżdża, niezależnie od stopnia zapełnienia.
* **SYGNAŁ 2 (np. SIGUSR2): Załadunek Ekspresowy**
    * Skierowany do: **Pracownika P4**.
    * Akcja: P4 wybudza się, blokuje dostęp do ciężarówki (mutex), ładuje jeden pakiet priorytetowy bezpośrednio na pakę i zasypia.
* **SYGNAŁ 3 (np. SIGINT/SIGTERM): Koniec Pracy**
    * Skierowany do: **Wszystkich procesów**.
    * Akcja: Pracownicy kończą pętle, ciężarówki zjeżdżają do bazy. Główny proces czyści pamięć dzieloną i usuwa semafory (`ipcrm`), a następnie generuje raport końcowy.

---

## 2. Architektura i Mechanizmy IPC

W projekcie wykorzystano natywne mechanizmy Systemu V:

### A. Pamięć Dzielona (`shmget`, `shmat`)
Przechowuje stan całego systemu, dostępny dla wszystkich procesów:
* Bufor cykliczny reprezentujący taśmę.
* Liczniki: aktualna waga na taśmie, liczba paczek, stan załadowania obecnej ciężarówki.

### B. Semafory (`semget`, `semop`, `semctl`)
Zestaw semaforów kontroluje dostęp do sekcji krytycznych i synchronizuje przepływ:
1.  **SEM_MUTEX:** Semafor binarny (zainicjowany na 1). Realizuje wzajemne wykluczanie (Mutual Exclusion) przy dostępie do pamięci dzielonej.
2.  **SEM_EMPTY:** Zlicza wolne miejsca na taśmie (blokuje pracowników, gdy taśma pełna).
3.  **SEM_FULL:** Zlicza paczki gotowe do odbioru (blokuje ciężarówkę, gdy taśma pusta).
4.  **SEM_WEIGHT:** Kontroluje dostępny udźwig taśmy (blokuje położenie paczki, jeśli przekroczono limit $M$).
5.  **SEM_TRUCK_READY:** Synchronizuje wymianę ciężarówek (zapewnia, że paczki są ładowane tylko gdy pojazd jest podstawiony).

---

## 3. Raportowanie
Po zakończeniu pracy program generuje plik `raport.txt` oraz czyści zasoby systemowe. Raport zawiera:
* Liczbę obsłużonych ciężarówek.
* Sumaryczną masę i objętość przewiezionych ładunków.
* Stan taśmy w momencie zatrzymania.

---

## 4. Plan Testów i Weryfikacja

System zostanie poddany testom weryfikującym poprawność synchronizacji procesów oraz obsługę sygnałów.

### Scenariusz A: Weryfikacja pojemności taśmy (Bufor Ograniczony)
**Cel:** Sprawdzenie, czy producenci (P1-P3) zatrzymują się, gdy taśma jest pełna (limit $K$) lub przeciążona (limit $M$).
1.  **Działanie:** Uruchomienie symulacji z dużą częstością generowania paczek przy powolnym odbiorze przez ciężarówkę.
2.  **Oczekiwany rezultat:** Komunikaty w logach: `[P1] Czekam na miejsce` lub `[P2] Czekam na udźwig`. Liczba paczek na taśmie nigdy nie przekracza $K$, a waga nie przekracza $M$.

### Scenariusz B: Obsługa priorytetu (P4 - Przesyłka Ekspresowa)
**Cel:** Sprawdzenie, czy P4 omija kolejkę taśmy.
1.  **Działanie:** Wysłanie sygnału nr 2 do dyspozytora w trakcie normalnej pracy.
2.  **Oczekiwany rezultat:** Pracownik P4 natychmiast blokuje dostęp do ciężarówki, ładuje pakiet i zwalnia zasób. Ciężarówka w tym czasie nie pobiera zwykłych paczek.

### Scenariusz C: Wymuszony odjazd ciężarówki
**Cel:** Weryfikacja obsługi sygnału odjazdu (Sygnał 1).
1.  **Działanie:** Wysłanie sygnału nr 1, gdy ciężarówka jest zapełniona np. w 50%.
2.  **Oczekiwany rezultat:** Ciężarówka odjeżdża natychmiast. Na jej miejsce po czasie $Ti$ podstawia się nowa (pusta).

### Scenariusz D: Poprawność FIFO i integralność danych
**Cel:** Sprawdzenie, czy paczki nie giną i zachowana jest kolejność.
1.  **Działanie:** Analiza raportu końcowego.
2.  **Oczekiwany rezultat:** Suma mas paczek wyprodukowanych musi równać się sumie mas paczek wywiezionych + paczek pozostałych na taśmie.

### Scenariusz E: Wycieki zasobów (Resource Leak)
**Cel:** Sprawdzenie, czy po zakończeniu programu (Sygnał 3) semafory są poprawnie usuwane.
1.  **Działanie:** Zakończenie programu i sprawdzenie stanu systemu poleceniem `ipcs`.
2.  **Oczekiwany rezultat:** Brak wiszących semaforów i segmentów pamięci przypisanych do użytkownika.