---
layout: cover
transition: fade
background: /img/header-bg.svg
---

# Wątki - std::thread

---

# Zarządzanie wątkami

<v-clicks>

* Klasa `std::thread` jest odpowiedzialna za tworzenie obiektów, które uruchamiają wątki systemowe i zarządzają nimi
* Każda instancja klasy `std::thread` reprezentuje
  - pojedynczy wątek, utworzony przez system operacyjny
  - lub tzw. **wątek pusty** (*not-a-thread*)

</v-clicks>

---

# Wątek pusty

<v-clicks>

* Instancja `std::thread` utworzona konstruktorem domyślnym 
* lub obiekt wątku po wykonaniu na nim operacji `std::move()`

</v-clicks>

<div v-click>

<<< @/../examples/01_threads_basics.cpp#empty_thread {all}

</div>

---

# std::thread + std::move

<v-clicks>

* Obiekty wątków nie są kopiowalne - *non-copyable*
* Wątki systemowe mogą być przenoszone między obiektami 
  - `std::thread` implementuje semantykę przenoszenia - *move semantics*
  - w celu transferu wątku, który jest l-value należy użyć funkcji `std::move`

</v-clicks>

---

# Tworzenie wątku - wskaźnik do funkcji

* Tworzenie wątku przez przekazanie wskaźnika do funkcji

<<< @/../examples/01_threads_basics.cpp#function_pointer {all|1-4|8-9}

---

# Tworzenie wątku - obiekt funkcyjny
 
*  Tworzenie wątku przez przekazanie obiektu funkcyjnego (funktora)

<div class="text-code-07">

<<< @/../examples/01_threads_basics.cpp#functor {all|1-13|17-21}

</div>

---

# Tworzenie wątku - obiekt domknięcia (lambda)

* Tworzenie wątku przez przekazanie obiektu domknięcia (lambdy)

<<< @/../examples/01_threads_basics.cpp#lambda

---

# Dołączenie do wątku

* Aby poczekać na zakończenie wykonywania zadania przez dany wątek, należy wywołać na jego rzecz metodę `join()`
* `join()` blokuje (wstrzymuje wywołanie) bieżący wątek, aż do czasu ukończenia zadania wykonywanego przez wskazany wątek
* Operacja `join()` jest **punktem synchronizacji** między wątkami
  * wszystkie zmiany dokonane w wątku dołączanym są widoczne w wątku dołączającym po zakończeniu `join()`

---

# Dołączenie do wątku

<<< @/../examples/01_threads_basics.cpp#join

---

# C++ Core Guidelines - join

[Prefer gsl::joining_thread over std::thread](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#rconc-joining_thread)

---

# Destruktor wątku

* Jeśli obiekt wątku reprezentuje aktywny wątek systemowy (*active thread of execution*), metoda `joinable()` zwraca `true`.
* Destrukcja obiektu wątku jest bezpieczna, jeśli obiekt wątku nie jest skojarzony z wątkiem systemowym. 
* W przeciwnym wypadku wywołana jest funkcja `std::terminate()`

---

# Destruktor wątku

* Obiekt wątku nie jest skojarzony z wątkiem systemowym:
  - jeśli został utworzony za pomocą konstruktora domyślnego
  - został przeniesiony do innego obiektu - np. jest po wywołaniu `std::move()`
  - została wcześniej wywołana operacja `join()`
  - została wcześniej wywołana operacja `detach()`

---

# Destrukcja wątku

* Standard C++ wymusza jawne wywołanie `join()` lub `detach()` przed wywołaniem destruktora wątku, który jest skojarzony z wątkiem systemowym.

---

# Przekazywanie parametrów do wątków

## 3 sposoby

---

# Sposób 1

* Przekazując argumenty jako kolejne (po funkcji lub obiekcie funkcyjnym) parametry konstruktora `std::thread`
* Podane argumenty są kopiowane lub przenoszone do uruchamianego wątku

<<< @/../examples/02_thread_args.cpp#by_value

---

# Przekazanie referencji

* Jeśli wymagane jest przekazanie referencji do funkcji uruchamianej w nowym wątku, należy użyć standardowych wraperów dla:
  * referencji - `std::ref()` 
  * referencji do stałej - `std::cref()`

---

# Przekazanie referencji - przykład

<<< @/../examples/02_thread_args.cpp#by_reference {all|1-5|9-15}

---

# Sposób 2

* Wykorzystując obiekt domknięcia, który przechwytuje zmienne będące argumentami wywoływanej funkcji

<div class="text-08">

<<< @/../examples/02_thread_args.cpp#by_closure {all|12}

</div>

---

# Sposób 3

* Konstrukcja obiektu funkcyjnego przekazywanego do konstruktora instancji `std::thread`:

<div class="text-07">

<<< @/../examples/01_threads_basics.cpp#functor {all}

</div>

---

# Problem wiszących referencji

* Przy przekazywaniu parametrów do wątków należy unikać wiszących referencji (dangling references)

[C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#rconc-data-by-value)

---

# std::thread i semantyka przenoszenia

* Klasa `std::thread` jest **move-only**
* Nie można kopiować obiektów `std::thread`, można je jedynie przenosić
* Obiekty `std::thread` można przechowywać w kontenerach, takich jak `std::vector`, pod warunkiem użycia semantyki przenoszenia

---

# std::thread i kontenery std

<div class="text-09">

* Wątki można grupować w kontenerach, takich jak `std::vector`, przy użyciu semantyki przenoszenia

</div>

<div class="text-sm">

<<< @/../examples/02_thread_args.cpp#move_semantics {all|3-7|14|16-18}

</div>

---
layout: cover
background: /img/header-bg.svg
---

# std::jthread - C++20

---

# std::jthread

* Ulepszona wersja `std::thread`, która:
  * wywołuje `join()` w destruktorze
  * obsługuje możliwość kooperatywnego anulowania wątku przez `std::stop_token`

---

# std::jthread

<div class="text-code-08">

<<< @/../examples/03_jthread.cpp#safe_join {all|15-16|18|20-21}

</div>

---

# std::jthread - destruktor

* Implementacja destruktora `std::jthread`:

```cpp
std::jthread::~jthread() 
{
    if (joinable()) 
    {
        request_stop(); // cooperative cancelling of the task
        join();
    }
}
```

---

# Kooperatywne anulowanie zadań

* Model kooperatywnego anulowania zadań może być uniwersalnie stosowany przez
  * wątki, korutyny, operacje sieciowe, I/O, itp.
* Nagłówek `<stop_token>`
* Podstawowe typy:
  * `std::stop_source` - obiekt *źródła* - umożliwia zażądania anulowania zadania
  * `std::stop_token` - obiekt umożliwiający sprawdzenie, czy zażądano anulowania zadania
  * `std::stop_callback` - callback w przypadku anulowania zadania

---

# std::stop_source / std::stop_token

* Implementacja zadania z obsługą kooperatywnego anulowania:

<div class="text-code-08">

<<< @/../examples/03_jthread.cpp#stop_token_task {all|1|7-12|14-15}

</div>

---

# std::stop_source / std::stop_token

* Uruchomienia zadania i jego anulowanie

<<< @/../examples/03_jthread.cpp#run_task_and_cancel {all|3-4|6-7|11}

---

# std::jthread - internal stop-state

* Jeśli nie przekażemy w wywołaniu konstruktora `std::jthread` jawnie tokena (a funkcja go oczekuje), to wykorzystany zostanie wewnętrzny stan obiektu wątku (*internal stop-state*)

<<< @/../examples/03_jthread.cpp#internal_stop_state

---

# std::stop_callback

* Obiekt RAII, umożliwiający rejestrację callbacku dla skojarzonego obiektu `std::stop_token`
* Callback zostanie wywołany, gdy skojarzony z obiektem `std::stop_token` obiekt `std::stop_source` zgłosi żądanie zatrzymania wątku
* Destruktor obiektu `std::stop_callback` automatycznie wyrejestruje callback, jeśli nie został on wcześniej wywołany
* Callback jest wywołany:
  *  w wątku, który zgłosił żądanie zatrzymania wątku
  *  lub w przypadku, gdy żądanie anulowania wątku zostało zgłoszone przed rejestracją callbacku, w wątku, w którym nastąpiła rejestracja callbacku

---

# std::stop_callback

<<< @/../examples/03_jthread.cpp#stop_callback

---
layout: cover
background: /img/header-bg.svg
---

# Wątki + wyjątki

---

# Obsługa wyjątków w wątkach

* Jeżeli z funkcji uruchomionej w osobnym wątku wydostanie się wyjątek, zostanie wywołana funkcja `std::terminate()`
* W celu prawidłowej obsługi wyjątków, należy przechwycić rzucony wyjątek i jeżeli istnieje taka potrzeba, przekazać go do wątku rodzica wykorzystując klasę `std::exception_ptr`

---

# Klasa std::exception_ptr

* Umożliwia przechowanie wskaźnika do obiektu wyjątku, który został zgłoszony instrukcją `throw` i przechwycony funkcją `std::current_exception()`
* Instancja `std::exception_ptr` może być przekazana do innej funkcji, również takiej, która uruchomiona jest w osobnym wątku
* Obsługa przekazanego przez wskaźnik wyjątku jest możliwa przy pomocy funkcji `std::rethrow_exception()`, która powoduje ponowne rzucenie wyjątku

---

# Klasa std::exception_ptr

* Domyślnie skonstruowany obiekt `std::exception_ptr` ma wartość `nullptr`
* Dwa obiekty są uznawane za równe, jeśli są puste lub wskazują na ten sam obiekt wyjątku
* Instancja `std::exception_ptr` jest konwertowalna do wartości logicznej

---

# Przechwytywanie wyjątku w wątku

<<< @/../examples/04_thread_exceptions.cpp#capture_exception {all|3-8|5|9-12}

---

# Obsługa wyjątku w wątku głównym

<div class="text-08">

<<< @/../examples/04_thread_exceptions.cpp#handle_exception {all|3|5|6|8-18}

</div>

---
layout: cover
background: /img/header-bg.svg
---

# Funkcje i klasy pomocnicze

---

# std::thread::hardware_concurrency()

* Statyczna metoda zwracająca ilość dostępnych wątków sprzętowych
* Zwykle podawana jest ilość procesorów, rdzeni, itp. Jeżeli informacja nie jest dostępna zwraca wartość `0`

<div v-click>

<<< @/../examples/05_thread_helpers.cpp#hardware_concurrency {all}

</div>

---

# Przestrzeń nazw std::this_thread

* `sleep_for(const chrono::duration<R, P>& sleep_duration)`
  - wstrzymuje wykonanie bieżącego wątku na (przynajmniej) określony interwał czasu
* `sleep_until(const chrono::time_point<C, D>& sleep_time)`
  - blokuje wykonanie wątku przynajmniej do podanego jako parametr punktu czasu
* `yield()`
  - funkcja umożliwiająca podjęcie próby wywłaszczenia bieżącego wątku i przydzielenia czasu procesora innemu wątkowi
* `get_id()` 
  - zwraca obiekt typu `std::thread::id` reprezentujący identyfikator bieżącego wątku

---

# Identyfikator wątku

* `std::thread::id` jest lekką, trywialnie kopiowalną klasą, która opakowuje unikalny identyfikator wątku
* Celem klasy jest możliwość użycia jej jako klucza w kontenerach asocjacyjnych

---

# Klasa std::thread::id

* Domyślny konstruktor tworzy obiekt identyfikujący tzw. pusty wątek (Not-A-Thread)
* Instancje `thread::id` są kopiowalne, porównywalne oraz hashowalne
  - w klasie zdefiniowany jest pełen zestaw operatorów porównania
  - biblioteka standardowa definiuje pomocniczą klasę `std::hash<std::thread::id>`, która umożliwia przechowanie wątku w kontenerach hashujących (np. `unordered_map`)
* Klasa posiada operator wyjścia do strumienia `operator<<`
