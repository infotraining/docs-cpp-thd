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

```c++
std::thread thd_empty; // not-a-thread

std::thread thd1(&some_function);
std::thread thd2 = std::move(thd1); // thd1 is now not-a-thread
thd2.join();
```
</v-clicks>

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

Tworzenie wątku przez przekazanie wskaźnika do funkcji

```cpp {all|1|3-6|10-11}
#include <thread>

void background_task()
{
    std::cout << "My first thread..." << std::endl;
}

int main()
{
    std::thread thd(&background_task);
    thd.join();
}
```

---

# Tworzenie wątku - obiekt funkcyjny
 
Tworzenie wątku przez przekazanie obiektu funkcyjnego (funktora)

<div class="text-code-07">
```cpp  {all|1-11|15-19} 
class BackgroundTask
{
    const int id_;
public:
    BackgroundTask(int id) : id_{id} {}

    void operator()() const
    {
        std::cout << "Background task " << id_ << " is running..." << std::endl;
    }
};

int main()
{
    BackgroundTask bt{1};
    std::thread thd_1(bt);
    std::thread thd_2(BackgroundTask{2});
    thd_1.join();
    thd_2.join();
}
```
</div>

---

# Tworzenie wątku - obiekt domknięcia (lambda)

Tworzenie wątku przez przekazanie obiektu domknięcia (lambdy)

```cpp
auto task = [] {
    std::cout << "My first task..." << std::endl;
};

std::thread thd_1(task);

std::thread thd_2([] { 
    std::cout << "My second task..." << std::endl; 
});

thd_1.join();
thd_2.join();
```

---

# Dołączenie do wątku

* Aby poczekać na zakończenie wykonywania zadania przez dany wątek, należy wywołać na jego rzecz metodę `join()`
* `join()` blokuje (wstrzymuje wywołanie) bieżący wątek, aż do czasu ukończenia zadania wykonywanego przez wskazany wątek
* Operacja `join()` jest **punktem synchronizacji** między wątkami
  * wszystkie zmiany dokonane w wątku dołączanym są widoczne w wątku dołączającym po zakończeniu `join()`

---

# Dołączenie do wątku

```cpp
int main()
{
    BackgroundTask bt{1};

    std::thread thd(bt);
    thd.join(); // wstrzymanie wykonania wątku głównego (main),
                // aż do czasu zakończenia zadania w wątku t

    assert(!thd.joinable());
}
```

---

# C++ Core Guidelines - join

[Prefer gsl::joining_thread over std::thread](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#rconc-joining_thread)

---

# Destruktor wątku

* Jeśli obiekt wątku jest skojarzony z wątkiem systemowym wywołanie metody `joinable()` zwraca `true`.
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

Standard C++ wymusza jawne wywołanie `join()` lub `detach()` przed wywołaniem destruktora wątku, który jest skojarzony z wątkiem systemowym.

---

# Przekazywanie parametrów do wątków

## 3 sposoby

---

# Sposób 1

* Przekazując argumenty jako kolejne (po funkcji lub obiekcie funkcyjnym) parametry konstruktora `std::thread`
* Podane argumenty są kopiowane lub przenoszone do uruchamianego wątku

```cpp
void do_work(int value, std::string text)
{
    //...
}

std::thread thd{&do_work, 42, "some data"};
thd.join();
```

---

# Przekazanie referencji

* Jeśli wymagane jest przekazanie referencji do funkcji uruchamianej w nowym wątku, należy użyć standardowych wraperów dla:
  * referencji - `std::ref()` 
  * referencji do stałej - `std::cref()`

---

# Przekazanie referencji - przykład

```cpp {all|1-5|1,9-15}
void do_work(ThreadSafeQueue<Data>& q, const std::string& value)
{
    Data data = process_data(value);
    q.push(data)
}

int main()
{
    ThreadSafeQueue<Data> data_queue;
    const std::string value = "some data";

    std::thread thd{&do_work, std::ref(data_queue), std::cref(value)};

    //...
    thd.join();
}
```

---

# Sposób 2

* Wykorzystując obiekt domknięcia, który przechwytuje zmienne będące argumentami wywoływanej funkcji

```cpp
void do_work(ThreadSafeQueue<Data>& q, std::string value)
{
    Data data = process_data(value);
    q.push(data)
}
```

```cpp {all|6}
int main()
{
    ThreadSafeQueue<Data> data_queue;
    const std::string value = "some data";

    std::thread thd{[&data_queue, value] { do_work(data_queue, value); }};

    thd.join();
}
```

---

# Sposób 3

* Konstrukcja obiektu funkcyjnego przekazywanego do konstruktora instancji `std::thread`:

```cpp
class BackgroundTask {
    const int id_;
public: 
    BackgroundTask(int id) : id_{id} {}

    void operator()() 
    {
        std::cout << "Hello from a task#" << id_ << std::endl;
    }
};

BackgroundTask background_task{1};
std::thread thd1{background_task};
std::thread thd2{BackgroundTask{2}};

thd1.join();
thd2.join();
```

---

# Problem wiszących referencji

Przy przekazywaniu parametrów do wątków należy unikać wiszących referencji (dangling references)

[C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#rconc-data-by-value)

---

# std::thread & move semantics

Klasa `std::thread` implementuje tylko semantykę przenoszenia

<div class="text-sm">
```cpp {all|3-7|12|15-16}
void task() { /* implementation */ }

std::thread create_thread() 
{
    std::thread thd(&task);
    return thd;
}

std::vector<std::thread> threads;

// implicit move of r-value
threads.push_back(create_thread());

// explicit move of l-value
std::thread thd(&task);
threads.push_back(std::move(thd));

for(auto& thd : threads)
    thd.join();
```
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
```cpp{all|16,21|17,22}
void background_work(const int id, const std::string text, 
                     const std::chrono::milliseconds delay) 
{
    std::cout << "Thread#" << id << " started..." << std::endl;

    for(const auto& letter : text)
    {
        std::cout << "Thread#" << id << " - " << letter << std::endl;
        std::this_thread::sleep_for(delay);
    }
}


void safe_join_in_cpp20() 
{
    std::jthread thd1{&background_work, 1, "Hello", 100ms};
    std::jthread thd2{&background_work, 2, "Multithreading", 250ms};
    
    may_throw(); 
    
    thd1.join(); // explicit join
} // implicit join in the destructor for thd2 & thd1 joins in case of exception
```
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

* Model kooperatywnego anulowania zadań może być uniwersalnie stosowany
  * wątki, korutyny, operacje sieciowe, I/O, itp.
* Nagłówek `<stop_token>`
* Podstawowe typy:
  * `std::stop_source` - obiekt *źródła* - umożliwia zażądania anulowania zadania
  * `std::stop_token` - obiekt umożliwiający sprawdzenie, czy zażądano anulowania zadania
  * `std::stop_callback` - callback w przypadku anulowania zadania

---

# std::stop_source / std::stop_token

Implementacja zadania:

```cpp {all|1|4-7,10-11,15}
void background_work(std::stop_token st, const int id, const std::string text) {
    std::cout << "Thread#" << id << " started..." << std::endl;

    for(const auto& letter : text)
    {
        if (st.stop_requested())
        {
            std::cout << "Stop has been requested..." << std::endl;
            std::cout << "Thread#" << id << " cancelled" << std::endl;
            return;
        }

        std::cout << "Thread#" << id << " - " << letter << std::endl;
        std::this_thread::sleep_for(250ms);
    }
}
```

---

# std::stop_source / std::stop_token

Uruchomienia zadania i jego anulowanie

```cpp {all|3-4|6-7|11|1-14}
void run_task_and_cancel()
{
    std::stop_source stop_src;
    std::stop_token st = stop_src.get_token();

    std::jthread thd1{&background_work, st, 1, "FIRST_TASK"};
    std::jthread thd2{&background_work, st, 2, "SECOND_TASK"};
    
    std::this_thread::sleep_for(2s);
    
    stop_src.request_stop();

    //...
}
```

---

# std::jthread - internal stop-state

Jeśli nie przekażemy w wywołaniu konstruktora `std::jthread` jawnie tokena (a funkcja go oczekuje), to wykorzystany zostanie wewnętrzny stan obiektu wątku (*internal stop-state*)

```cpp
void background_work(std::stop_token st, 
                     const int id, const std::string text);

std::jthread thd1{&background_work, 1, "TASK"}; // missing stop-token

std::stop_source stop_src = thd1.get_stop_source(); // get stop_source from jthread
std::stop_token st = thd1.get_stop_token();

stop_src.request_stop(); // or thd1.request_stop();
```

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

```cpp {all|3-12|5|9-12}
void background_work(std::exception_ptr& excpt)
{
    try
    {
        may_throw();

        // ...
    }
    catch(...)
    {
        excpt = std::current_exception();
    }
}
```

---

# Obsługa wyjątku w wątku głównym

```cpp {all|3|5|6|8-16}
int main()
{
    std::exception_ptr thd_exception;

    std::thread thd(&background_work, std::ref(thd_exception));
    thd.join();

    if (thd_exception)
    {
        try
        {
            std::rethrow_exception(thd_exception);
        }
        catch(const std::runtime_error& e)
        {
            // ... handling an error
        }
    }
}
```

---
layout: cover
background: /img/header-bg.svg
---

# Funkcje i klasy pomocnicze

---

# std::thread::hardware_concurrency()

* Statyczna metoda zwracająca ilość dostępnych wątków sprzętowych
* Zwykle podawana jest ilość procesorów, rdzeni, itp. Jeżeli informacja nie jest dostępna zwraca wartość `0`

<v-click>

```cpp
auto hardware_threads_count = 
    std::max(1u, std::thread::hardware_concurrency());

std::vector<std::thread> threads(hardware_threads_count);
```

</v-click>

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
