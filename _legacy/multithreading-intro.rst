********************************************
Wprowadzenie do programowania wielowątkowego
********************************************

Co to jest współbieżność
========================

Najprościej

* dwa działania lub więcej, wykonywane jednocześnie

Jeden procesor

* szybkie przełączanie zadań - iluzja równoległości

.. image:: _images/1-core.*

Wiele procesorów

* Sprzętowa równoległość

  - idealna sytuacja (*parallel execution, hardware threads*)
  
    .. image:: _images/2-cores-ideal.*      

  - realnie (*threads oversubscription*)
    
    .. image:: _images/2-cores-real.*
   

Proces
======

* Egzemplarz wykonywanego programu
* Kontener wątków w chronionej przestrzeni adresowej
* System operacyjny przydziela procesowi (alokuje) odpowiednie zasoby
* Za zarządzanie procesami odpowiada jądro systemu operacyjnego

  - system operacyjny zarządza priorytetami procesów

W systemach wielozadaniowych procesy mogą być wykonywane współbieżnie:

* systemy wieloprocesorowe lub wielordzeniowe - współbieżność rzeczywista
* jednoprocesorowe - emulacja współbieżności

  - z wywłaszczaniem - pre-emptive multitasking
  - bez wywłaszczania


Sposoby komunikacji między procesami
------------------------------------

Komunikacja między procesami:

* Sygnały
* Gniazda (*sockets*)
* Pliki
* Potoki
* Pamięć współdzielona


Wątek
=====

**Wątek**
  niezależny ciąg instrukcji wykonywany współbieżnie w ramach jednego procesu

Wszystkie wątki działające w danym procesie współdzielą przestrzeń adresową oraz zasoby systemowe

* pamięć (*heap*, *static storage*)
* pliki wymagane przez aplikację
* cykle CPU (*multitasking*)
* gniazda (*sockets*)

.. image:: _images/threads.*
   :align: center

Zasobem, który wątki nie współdzielą jest pamięć stosu (*stack*). Każdy utworzony wątek posiada własny stos, na którym alokowane są lokalne zmienne funkcji wywołanej w tym wątku.

Wątki są udostępniane i kontrolowane przez system operacyjny:

* MS Windows – Win32 API
* Linux, BSD - pthread


Proces czy wątek?
=================

Procesy:

* są niezależne (izolowane)
* mają oddzielne przestrzenie adresowe
* komunikują się ze sobą za pośrednictwem IPC
* proces może zostać uruchomiony na innym komputerze (lepsza skalowalność)
* jeśli stabilność jest istotna, należy użyć procesów
* jeśli wykorzystywane są zasoby, które są dostępne tylko na zasadzie „jeden dla procesu”, należy wybrać proces
  
Wątki:

* należą do procesu
* dzielą przestrzenie adresowe - z wyjątkiem stosu
* przełączania kontekstu wątków w tym samym procesie jest zwykle szybsze niż między procesami
* komunikacja wewnątrz wątków jest mniej kosztowna niż komunikacja międzyprocesowa
* jeśli wątki dzielą zasoby, które nie mogą być użyte przez wiele procesów jednocześnie, należy korzystać z wątków
  - albo zapewnić IPC

Jedną z największych różnic między wątkami a procesami jest to, że wątki wykorzystują konstrukcje oprogramowania do ochrony struktur danych, procesy używają sprzętu.


Po co używać współbieżności
===========================

* Rozdzielenie odpowiedzialności

  - Grupowanie powiązanego kodu
  - Rozdzielanie odrębnych operacji, gdy mają się odbywać w tym samym czasie


* Wydajność

  - Chcemy wykorzystać całą moc maszyny wieloprocesorowej
  - Chcemy podnieść skalowalność aplikacji 
  - *"The free lunch is over"*
  - Podział zadań na części i wykonywanie równolegle (dekompozycja problemu może być trudna)


* Lepsza responsywność aplikacji
  
  - Unikanie blokujących operacji I/O
  - Blokowanie interfejsu użytkownika


Kiedy nie używać współbieżności?
================================

Należy unikać wielowątkowości, gdy korzyści nie są warte kosztów:

* Kod wielowątkowy jest dużo bardziej skomplikowany - trudniejszy do pisania, czytania i testowania niż kod jednowątkowy
* Większa złożoność, więcej błędów, które są trudne do wykrycia (często trudne do odtworzenia)
* Narzuty związane z zarządzaniem wątkami mogą drastycznie obniżyć wydajność (context switching, cache ping-pong, false sharing)
* Zbyt wiele wątków jest uruchomionych jednocześnie

  - zużycie zasobów systemu operacyjnego
  - system jako całość będzie działał wolniej


Testowanie aplikacji wielowątkowych
===================================

* Kod jednowątkowy może być testowany jednostkowo (unit tests)

  - Powtarzalne wyniki dla testów wykonywanych w izolacji


* Kod wielowątkowy jest trudno testowalny jednostkowo

  - Błędy (np. *race conditions*) nie są deterministyczne i są ciężko reprodukowalne
  - Problemy pojawiają często przy dużym obciążeniu lub zwielokrotnieniu wątków
    
* Dobrze jest zapewnić możliwość skalowania ilości wątków do jednego (single threaded
  code) aby wykluczyć inne przyczyny błędów
