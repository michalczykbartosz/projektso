#!/bin/bash


# Liczba powtórzeń testu

LICZBA_PROB=20

# Liczba ciężarówek w symulacji (im więcej, tym łatwiej wyłapać błąd)

ILE_CIEZAROWEK=5


echo "=== ROZPOCZYNAM STRESS-TEST (20 prób) ==="

echo "Cel: Sprawdzenie, czy procesy przeżywają SIGCONT"


passed=0

failed=0


for (( i=1; i<=LICZBA_PROB; i++ ))

do

    echo "----------------------------------------"

    echo "PRÓBA #$i"

    

    # 1. Uruchomienie programu w tle

    ./main $ILE_CIEZAROWEK > /dev/null 2>&1 & 

    MAIN_PID=$!

    

    # 2. Czekamy 2 sekundy, żeby procesy się namnożyły i zaczęły pracę

    sleep 2


    # Sprawdźmy ile jest ciężarówek przed zatrzymaniem

    liczba_przed=$(pgrep -f "ciezarowka" | wc -l)

    

    if [ "$liczba_przed" -eq 0 ]; then

        echo "BŁĄD: Symulacja w ogóle nie ruszyła!"

        killall -9 main pracownik ciezarowka 2>/dev/null

        exit 1

    fi


    echo " -> Zamrażam (SIGSTOP)..."

    kill -STOP $MAIN_PID 2>/dev/null

    pkill -STOP -P $MAIN_PID 2>/dev/null # Zamroź dzieci maina

    pkill -STOP -f "ciezarowka" 2>/dev/null

    pkill -STOP -f "pracownik" 2>/dev/null


    sleep 1


    echo " -> Wznawiam (SIGCONT)..."

    kill -CONT $MAIN_PID 2>/dev/null

    pkill -CONT -P $MAIN_PID 2>/dev/null

    pkill -CONT -f "ciezarowka" 2>/dev/null

    pkill -CONT -f "pracownik" 2>/dev/null


    # 3. Dajemy chwilę na ewentualną "śmierć" ciężarówki (jeśli błąd występuje)

    sleep 1


    # 4. Weryfikacja

    liczba_po=$(pgrep -f "ciezarowka" | wc -l)


    if [ "$liczba_po" -lt "$liczba_przed" ]; then

        echo -e "\033[0;31m[FAIL]\033[0m Ciężarówka zniknęła! (Było: $liczba_przed, Jest: $liczba_po)"

        ((failed++))

    else

        echo -e "\033[0;32m[OK]\033[0m Wszystkie ciężarówki żyją."

        ((passed++))

    fi


    # 5. Sprzątanie przed kolejną próbą (wysyłamy SIGINT żeby posprzątał IPC)

    kill -INT $MAIN_PID 2>/dev/null

    sleep 1

    # Dla pewności dobijamy resztki, jeśli main nie zdążył

    killall -9 main pracownik ciezarowka 2>/dev/null

    # Czekamy chwilę, żeby system zwolnił zasoby

    sleep 1

done


echo "========================================"

echo "PODSUMOWANIE:"

echo -e "\033[0;32mUDANE: $passed\033[0m"

if [ "$failed" -gt 0 ]; then

    echo -e "\033[0;31mNIEUDANE: $failed\033[0m"

else

    echo "Perfekcyjnie! Wszystkie testy przeszły."

fi
