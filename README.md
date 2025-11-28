# Projekt Systemy Operacyjne - Temat 10: Magazyn Firmy Spedycyjnej.

**Imię:** Bartosz **Nazwisko:** Michalczyk **Nr. Albumu:** 156487 **Github:** https://github.com/michalczykbartosz/projektso

## 1. Pracownicy i Przesyłki
W magazynie przy taśmie transportowej pracuje trzech pracowników (P1, P2, P3). Układają oni paczki o następujących gabarytach:

| Pracownik | Typ Paczki | Wymiary (cm) |
| :---:     | :---:      | :---:        |
| **P1**    | **A**      | 64 x 38 x 8  |
| **P2**    | **B**      | 64 x 38 x 19 |
| **P3**    | **C**      | 64 x 38 x 41 |

* **Waga:** Maksymalnie 25 kg (wartość losowa z zakresu 0,1 kg – 25,0 kg).
* **Założenie:** Czym mniejsza paczka, tym mniejszy ciężar.

## 2. Przesyłki Ekspresowe (Pracownik P4)
* Pracownik **P4** odpowiada za załadunek ekspresowy.
* Przesyłki te dostarczane są osobno (nie trafiają na taśmę).
* **Priorytet:** Mają wyższy priorytet – ich załadunek musi odbyć się w pierwszej kolejności.
* Pakiet ekspresowy może zawierać paczki A, B i C o wadze < 25 kg.

## 3. Taśma Transportowa i Ciężarówki
Na końcu taśmy stoi ciężarówka, którą należy załadować do pełna.

**Parametry Ciężarówek:**
* **Ładowność:** W [kg]
* **Pojemność:** V [m3]
* **Liczba ciężarówek:** N
* **Cykl pracy:** Po załadowaniu ciężarówka rozwozi towar i wraca po czasie **Ti**. Po odjeździe pełnej ciężarówki, natychmiast podstawia się nowa (jeśli jest dostępna).

**Ograniczenia Taśmy:**
* **Liczba przesyłek:** Maksymalnie **K** przesyłek w danej chwili.
* **Udźwig:** Maksymalnie **M** jednostek masy.
* **Zasada:** Niedopuszczalne jest przekroczenie udźwigu (np. same najcięższe paczki).
* **Kolejność:** Przesyłki muszą trafić na samochód w takiej samej kolejności, w jakiej zostały położone na taśmie (FIFO).

## 4. Sterowanie (Sygnały Dyspozytora)
Dyspozytor steruje pracą magazynu za pomocą sygnałów:

1.  **Sygnał 1:** Ciężarówka stojąca przy taśmie odjeżdża z niepełnym ładunkiem.
2.  **Sygnał 2:** Pracownik P4 dostarcza pakiet przesyłek ekspresowych do ciężarówki.
3.  **Sygnał 3:** Koniec pracy. Pracownicy kończą układanie, ciężarówki kończą pracę po rozwiezieniu wszystkich przesyłek.

## 5. Plan Testów i Weryfikacja

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
2.  **Oczekiwany rezultat:** Ciężarówka odjeżdża natychmiast (zwalnia zasób). Na jej miejsce **natychmiast** podstawia się nowa ciężarówka jeżeli jest dostępna.

### Scenariusz D: Poprawność FIFO i integralność danych
**Cel:** Sprawdzenie, czy paczki nie giną i zachowana jest kolejność.
1.  **Działanie:** Analiza raportu końcowego.
2.  **Oczekiwany rezultat:** Suma mas paczek wyprodukowanych musi równać się sumie mas paczek wywiezionych + paczek pozostałych na taśmie.

### Scenariusz E: Wycieki zasobów (Resource Leak)
**Cel:** Sprawdzenie, czy po zakończeniu programu (Sygnał 3) semafory są poprawnie usuwane.
1.  **Działanie:** Zakończenie programu i sprawdzenie stanu systemu poleceniem `ipcs`.
2.  **Oczekiwany rezultat:** Brak wiszących semaforów i segmentów pamięci przypisanych do użytkownika.