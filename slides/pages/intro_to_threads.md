---
layout: cover
background: /img/header-bg.svg
---

# Concurrent vs. Parallel

---
layout: default
---
 
# Concurrency vs. Parallelism

* **Concurrency** is about dealing with lots of things at once.
* **Parallelism** is about doing lots of things at once.

* Not the same, but related.
  * One is about structure, one is about execution.
  * Concurrency provides a way to structure a solution to solve a problem that may (but not necessarily) be parallelizable.

> -- Rob Pike, co-inventor of the Go language

---

# Jak osiągnąć współbieżność?

* Multiprocessing
  * wykorzystanie wielu procesów kontrolowanych przez system operacyjny
* Multithreading
  * wykorzystanie wielu wątków w ramach jednego procesu
* Wektoryzacja SIMD - Single Instruction, Multiple Data
  * nowoczesne procesory umożliwiają wykonywanie jednej instrukcji na wielu danych jednocześnie
* Korutyny
  * umożliwiają zawieszenie i wznowienie wykonywania funkcji w określonych punktach
  * umożliwiają efektywne zarządzanie operacjami (I/O) 

---
layout: cover
background: /img/header-bg.svg
---

# Proces vs. Wątek

---

# Proces

* Egzemplarz wykonywanego programu z własną przestrzenią adresową
* System operacyjny przydziela procesowi odpowiednie zasoby
  - pamięć
  - deskryptory plików
  - zasoby sieciowe
  - handlery sygnałów
* Za zarządzanie procesami odpowiada jądro systemu operacyjnego
  - system operacyjny zarządza priorytetami procesów i decyduje o przydziale czasu procesora (CPU time-slicing)
* Izolowany od innych procesów - awaria jednego procesu nie wpływa na działanie innych procesów

---

# Komunikacja między procesami

* Sygnały
* Pamięć współdzielona
* Pliki
* Potoki
* Gniazda (sockets)
* Kolejki - *message queues*

---

# Wątek

niezależny ciąg instrukcji wykonywany współbieżnie w ramach jednego procesu

---

# Wątki i zasoby

* Każdy wątek posiada własny stos (*stack*) 
* Kontroluje stan rejestrów procesora
* Wszystkie wątki działające w danym procesie współdzielą przestrzeń adresową oraz zasoby systemowe
  - pamięć (heap, static storage)
  - pliki otwarte przez aplikację
  - cykle CPU (multitasking)
  - gniazda (sockets)

---

# Natywne API dla wątków

* Wątki są zarządzane przez system operacyjny
* Konkretny interfejs API zależy od systemu operacyjnego
  * MS Windows - Win32 API
  * Linux, BSD - pthread

---

# Cross-platform C++ API

* Boost.Thread
* Threading Building Blocks - Intel
* Standard C++ Library - od C++11


---

# GCC - opcje kompilacji

```bash
g++ -std=c++11 -pthread main.cpp
```

---

# Po co używać współbieżności

<v-clicks>

* Wydajność
  - Chcemy wykorzystać całą moc maszyny wieloprocesorowej
  - Chcemy podnieść skalowalność aplikacji
  - „The free lunch is over"
* Lepsza responsywność aplikacji
  - Unikanie blokujących operacji I/O
  - Blokowanie interfejsu użytkownika

</v-clicks>

---

# Kiedy nie używać współbieżności

<v-clicks>

* Gdy korzyści nie są warte kosztów
* Potencjalne problemy:
  * Kod wielowątkowy jest dużo bardziej skomplikowany 
    - trudniejszy do pisania, czytania i testowania niż kod jednowątkowy 
  * Większa złożoność, więcej błędów, które są trudne do wykrycia (często trudne do odtworzenia) 
  * Narzuty związane z zarządzaniem wątkami mogą drastycznie obniżyć wydajność 
    - thread oversubsription + context switching 
    - cache ping-pong 
    - false sharing 

</v-clicks>


---

# Testowanie aplikacji wielowątkowych

<v-clicks>

* Kod jednowątkowy może być testowany jednostkowo (unit tests) 
  - powtarzalne wyniki dla testów wykonywanych w izolacji
* Kod wielowątkowy jest trudno testowalny jednostkowo 
  - błędy (np. race conditions) nie są deterministyczne i są ciężko reprodukowalne
  - problemy pojawiają często przy dużym obciążeniu lub zwielokrotnieniu wątków
* Dobrze jest zapewnić możliwość skalowania ilości wątków do jednego (single threaded code) aby wykluczyć inne przyczyny błędów


</v-clicks>


