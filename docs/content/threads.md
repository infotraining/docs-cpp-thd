# Wątki

## Zarządzanie wątkami

* Klasa `std::thread` umożliwia uruchamianie wątków systemowych i zarządzanie nimi.
* Każdy obiekt `std::thread` reprezentuje aktywny wątek albo wątek pusty (*not-a-thread*). Wątek pusty reprezentuje obiekt utworzony konstruktorem domyślnym lub obiekt, z którego przeniesiono wątek.
* Obiekty `std::thread` nie są kopiowalne, ale można je przenosić. Aby przenieść wątek z obiektu będącego l-wartością, użyj `std::move()`.

## Tworzenie i uruchamianie wątków

Wątek jest uruchamiany przez przekazanie do konstruktora obiektu `std::thread` argumentu w postaci:

### Wskaźnika do funkcji

```cpp
#include <thread>
#include <iostream>

void background_worker()
{
    std::cout << "My first thread..." << std::endl;
}

int main()
{
    std::thread thd{&background_worker};
    thd.join();
}
```

### Obiektu funkcyjnego

```cpp
#include <thread>
#include <iostream>

class BackgroundWorker
{
public:
    void operator()() const
    {
        std::cout << "Hello from a thread..." << std::endl;
    }
};

int main()
{
    BackgroundWorker worker;
    std::thread thd1{worker};
    std::thread thd2{BackgroundWorker{}};
    thd1.join();
    thd2.join();
}
```

### Funkcji lambda

```cpp
#include <thread>
#include <iostream>

int main()
{
    std::thread thd([] { std::cout << "My first thread..." << std::endl; });
    thd.join();
}
```

## Dołączenie do wątku

Aby poczekać na zakończenie pracy wątku, wywołaj na jego obiekcie metodę `join()`. Metoda ta blokuje bieżący wątek do chwili zakończenia wskazanego wątku.

```cpp
int main()
{
    BackgroundWorker worker;

    std::thread thd(worker);
    thd.join(); // Block main until thread thd finishes.

    assert(!thd.joinable());
}
```

## Odłączanie wątków od obiektów

Po wywołaniu `detach()` wątek działa niezależnie od obiektu `std::thread`, który go uruchomił. Taki wątek bywa nazywany **wątkiem w tle**.

Odłącz wątek, wywołując na jego obiekcie metodę `detach()`.

```cpp
std::thread thd(&background_worker);

thd.detach();

assert(!thd.joinable());
```

Zgodnie z wytycznymi [C++ Core Guidelines [CP.26]](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#cp26-dont-detach-a-thread) nie jest zalecane odłączanie wątków za pomocą ``detach()``.

## Przekazywanie parametrów do wątków

Przekazanie parametrów do uruchamianych wątków może odbywać się na trzy sposoby:

* Korzystając z parametrów konstruktora obiektu funkcyjnego przekazywanego do konstruktora obiektu ``std::thread``

```cpp
class BackgroundWorker
{
    int x_;
    double y_;

public:
    BackgroundWorker(int x, double y) : x_{x}, y_{y} {}

    void operator()()
    {
        // ... use the x_ and y_ members
    }
};

//...
BackgroundWorker worker(1, 3.14);
std::thread thd(worker);
thd.join();
```

* Przekazując argumenty jako kolejne parametry konstruktora `std::thread`, po funkcji lub obiekcie funkcyjnym. Argumenty są przechowywane w obiekcie wątku jako wartości: l-wartości są kopiowane, a r-wartości przenoszone.
  
  Aby przekazać referencję do funkcji uruchamianej w nowym wątku, użyj opakowania referencyjnego `std::ref()` albo `std::cref()` (dla referencji do stałej).

 ```cpp
void worker_by_val(int n)
{
    for (int i = 0; i < 5; ++i)
    {
        std::cout << "Thread 1: n = " << n << '\n';
        ++n; // Changing this local copy does not change n in main().
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
    
void worker_by_ref(int& n)
{
    for (int i = 0; i < 5; ++i) 
    {
        std::cout << "Thread 2 is modifying n\n";
        ++n;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
    
int main()
{
    int n = 0;
    
    std::thread thd1{&worker_by_val, n + 1};       // Pass by value.
    std::thread thd2{&worker_by_ref, std::ref(n)}; // Pass by reference.
    
    thd1.join();
    thd2.join();
    
    std::cout << "Final value of n: " << n << '\n';
}
```

* Wykorzystując obiekt domknięcia, który przechwytuje zmienne będące argumentami wywoływanej funkcji

```cpp
void background_worker(int index, std::vector<int>& data)
{
    const int calculated_value = index * 2;
    data[index] = calculated_value;
}

//...

std::vector<int> vec(2);

std::thread thd1{
    static_cast<void (*)(int, std::vector<int>&)>(&background_worker),
    0, std::ref(vec)};
std::thread thd2{[&vec] { background_worker(1, vec); }};

thd1.join();
thd2.join();
```

Oba zadania zapisują do różnych elementów istniejącego wektora. Nie należy zmieniać jego rozmiaru, dopóki te zapisy trwają.

### Problem wiszących referencji

Przy przekazywaniu parametrów do wątków należy unikać wiszących referencji (*dangling references*) - [C++ Core Guidelines [CP.31]](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#cp31-pass-small-amounts-of-data-between-threads-by-value-rather-than-by-reference-or-pointer).

```cpp
class BackgroundWorker
{
    int& value_;

public:
    explicit BackgroundWorker(int& value) : value_{value} {}

    void operator()()
    {
        do_something(value_);
    }
};

std::thread create_thread()
{
    int local_value = 0;
    BackgroundWorker worker(local_value);

    std::thread thd(worker);
    return thd;
} // local_value is destroyed, leaving the reference dangling.
```

## Transferowanie wątków

Obiekty wątków można przenosić zgodnie z zasadami **semantyki przenoszenia**. Do jawnego przeniesienia wątku z l-wartości użyj `std::move()`. Zwrócenie lokalnego obiektu będącego r-wartością z funkcji nie wymaga jawnego wywołania `std::move()`.

```cpp
void background_worker()
{
    /* ... */
}

std::thread create_thread()
{
    std::thread thd(&background_worker);
    return thd;
}
```

Wątki mogą być grupowane i przechowywane w kontenerach standardowych:

```cpp
std::thread thd{&background_worker};

std::vector<std::thread> threads(2);
threads[0] = std::move(thd);
threads[1] = create_thread();
threads.push_back(create_thread());

// Do not forget to join every joinable thread.
for (auto& thread : threads)
{
    if (thread.joinable())
        thread.join();
}
```

## Destruktor wątku

Destruktor obiektu `std::thread` wywołuje `std::terminate()`, jeśli obiekt nadal jest skojarzony z wątkiem systemowym (*joinable*). Przed zniszczeniem obiektu trzeba więc wywołać `join()` albo `detach()`.

Obiekt nie jest skojarzony z wątkiem systemowym, gdy:

* został utworzony konstruktorem domyślnym;
* jego wątek został przeniesiony do innego obiektu;
* na obiekcie wywołano wcześniej `join()` albo `detach()`.

Metoda `joinable()` zwraca `true`, jeśli obiekt nadal jest skojarzony z wątkiem systemowym.

```{warning}
Trzeba jawnie zdecydować, czy poczekać na zakończenie wątku (`join()`), czy odłączyć go od obiektu (`detach()`). Jeśli żadna z tych operacji nie nastąpi przed zniszczeniem obiektu, program zakończy działanie przez `std::terminate()`.

Ręczne pilnowanie tych operacji może być zawodne, zwłaszcza gdy wystąpi wyjątek. Warto używać obiektów RAII, które zarządzają czasem życia wątku. C++20 udostępnia `std::jthread`, który automatycznie żąda zatrzymania i dołącza wątek w destruktorze.
```

## Klasa std::jthread (C++20)

Klasa `std::jthread` to ulepszona wersja `std::thread`, która:

* w destruktorze zgłasza żądanie zatrzymania, a następnie dołącza wątek, jeśli jest joinable;
* obsługuje kooperatywne zatrzymywanie za pomocą `std::stop_token`.

``` cpp
void safe_join_in_cpp20()
{
    std::jthread thd1{&background_worker, 1, "Hello", 100ms};
    std::jthread thd2{&background_worker, 2, "Multithreading", 250ms};
    
    may_throw();
    
    thd1.join(); // Explicit join; thd2 is joined by its destructor.
    
} // If an exception is thrown, the destructors join any joinable threads.
```

W tym przykładzie `background_worker`, `may_throw` i literał `100ms`/`250ms` muszą być wcześniej zdefiniowane; literały wymagają `using namespace std::chrono_literals;`.

### Destruktor jthread

Schemat działania destruktora `std::jthread`:

```cpp
if (joinable()) {
    request_stop();
    join();
}
```

## Kooperatywne przerywanie (anulowanie) wątków

* Model kooperatywnego zatrzymywania zadań można stosować w różnych obszarach, między innymi wątkach, korutynach i operacjach I/O.
* Typy zatrzymywania wątków są zdefiniowane w nagłówku `<stop_token>`.
* Podstawowe typy:
  * `std::stop_source` — źródło, które może zgłosić żądanie zatrzymania;
  * `std::stop_token` — token, za pomocą którego zadanie może sprawdzić, czy zgłoszono żądanie zatrzymania;
  * `std::stop_callback` — callback wywoływany po zgłoszeniu żądania zatrzymania.

### Implementacja zadania

Funkcja zadania może przyjąć `std::stop_token` przez wartość i okresowo sprawdzać, czy zgłoszono żądanie zatrzymania:

``` cpp
void background_worker(std::stop_token token, int id, const std::string& text,
                       std::chrono::milliseconds delay = std::chrono::milliseconds{250})
{
    std::cout << "Thread#" << id << " started..." << std::endl;

    for (const auto& letter : text)
    {
        if (token.stop_requested())
        {
            std::cout << "Stop has been requested..." << std::endl;
            std::cout << "Thread#" << id << " cancelled" << std::endl;
            return;
        }

        std::cout << "Thread#" << id << " - " << letter << std::endl;
        std::this_thread::sleep_for(delay);
    }
}
```

Token pozwala bezpiecznie sprawdzić, czy zgłoszono żądanie zatrzymania. Zadanie samo decyduje, kiedy na nie zareagować — na przykład kończąc funkcję po najbliższym sprawdzeniu.

### Uruchomienie zadania i zgłoszenie żądania zatrzymania

Do tworzenia tokenów i zgłaszania żądania stopu służy obiekt typu `std::stop_source`:

```cpp
void run_task_and_cancel()
{
    std::stop_source shared_stop_source;
    std::stop_token shared_stop_token = shared_stop_source.get_token();

    // std::jthread automatically passes its own token to a callable
    // that accepts std::stop_token. The lambdas share one token instead.
    std::jthread thd1{[shared_stop_token] {
        background_worker(shared_stop_token, 1, "FIRST_TASK");
    }};
    std::jthread thd2{[shared_stop_token] {
        background_worker(shared_stop_token, 2, "SECOND_TASK");
    }};
    
    std::this_thread::sleep_for(std::chrono::seconds{2});
    
    shared_stop_source.request_stop();
}
```

Każda lambda przechwytuje ten sam token, więc żądanie ze `shared_stop_source` jest widoczne dla obu zadań. Destruktory obiektów `std::jthread` nadal dołączą wątki, jeśli są joinable.

#### Wewnętrzny stan zatrzymania `std::jthread`

Jeśli przekazana funkcja przyjmuje `std::stop_token` jako pierwszy argument i nie przekażemy go jawnie, konstruktor `std::jthread` przekaże token z własnego stanu zatrzymania obiektu.

```cpp
void background_worker(std::stop_token token, int id, const std::string& text,
                       std::chrono::milliseconds delay);

//...                     

std::jthread thd1{&background_worker, 1, "TASK", std::chrono::milliseconds{250}}; // The token is passed automatically.

std::stop_source source = thd1.get_stop_source();
std::stop_token token = thd1.get_stop_token();

source.request_stop(); // Equivalent to: thd1.request_stop().
```

## Obsługa wyjątków w wątkach

Jeśli wyjątek wydostanie się z funkcji uruchomionej w osobnym wątku, zostanie wywołana funkcja `std::terminate()`.

Jeśli wyjątek ma zostać obsłużony w innym wątku, przechwyć go i przekaż za pomocą `std::exception_ptr`.

`std::exception_ptr` przechowuje współdzielone odwołanie do wyjątku przechwyconego przez `std::current_exception()`. Można przekazać go do innego wątku, a następnie ponownie rzucić wyjątek za pomocą `std::rethrow_exception()`.

Instancje `std::exception_ptr` posiadają następujące cechy:
* domyślnie skonstruowany obiekt ``std::exception_ptr`` ma wartość ``nullptr``.
* dwa obiekty są uznawane za równe, jeśli są puste lub wskazują na ten sam obiekt wyjątku.
* instancja ``std::exception_ptr`` jest konwertowalna do wartości logicznej

```cpp
void background_worker(std::exception_ptr& excpt)
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

// ...

int main()
{
    std::exception_ptr thd_exception;

    std::thread thd{&background_worker, std::ref(thd_exception)};

    thd.join();

    try
    {
        if (thd_exception)
            std::rethrow_exception(thd_exception);
    }
    catch (const std::runtime_error& e)
    {
        // ... handling an error
    }
}
```

## Funkcje i klasy pomocnicze w bibliotece standardowej

### std::thread::hardware_concurrency()

Statyczna metoda zwracająca wskazówkę dotyczącą liczby wątków, które sprzęt i implementacja mogą wykonywać współbieżnie. Wartość może odpowiadać liczbie rdzeni lub logicznych procesorów; jeśli nie można jej ustalić, metoda zwraca `0`.

```cpp
    std::cout << "number of cores = "
              << std::thread::hardware_concurrency() << std::endl;
```

W praktyce często używa się kontenerów wątków o rozmiarze równym ilości wątków sprzętowych:

```cpp
auto hardware_threads_count = std::max(1u, std::thread::hardware_concurrency());
std::vector<std::thread> threads(hardware_threads_count);
```

### Przestrzeń nazw std::this_thread

Przestrzeń nazw `std::this_thread` zawiera funkcje pomocnicze, między innymi:

`sleep_for(const std::chrono::duration<Rep, Period>& sleep_duration)`
: wstrzymuje wykonanie bieżącego wątku na co najmniej określony interwał czasu

```cpp
using namespace std::chrono_literals;

std::cout << "Hello waiter" << std::endl;

auto start = std::chrono::steady_clock::now();

std::this_thread::sleep_for(2s);

auto end = std::chrono::steady_clock::now();

std::chrono::duration<double, std::milli> elapsed = end-start;
std::cout << "Waited " << elapsed.count() << " ms\n";
```

* `sleep_until(const std::chrono::time_point<Clock, Duration>& sleep_time)` — blokuje wykonanie wątku co najmniej do podanego punktu czasu.
  
* `yield()` — sugeruje schedulerowi, że bieżący wątek może oddać procesor innemu wątkowi; nie gwarantuje przełączenia.

* `get_id()` — zwraca obiekt `std::thread::id` reprezentujący identyfikator bieżącego wątku.

### Identyfikator wątku - std::thread::id

Klasa `std::thread::id` przechowuje identyfikator wątku, który można wykorzystać jako klucz w kontenerach asocjacyjnych, takich jak `std::map` i `std::unordered_map`.

```cpp
std::thread::id current_id = std::this_thread::get_id();
```

Domyślnie skonstruowany `std::thread::id` oznacza brak wątku. Identyfikatory można kopiować, porównywać, wyświetlać w strumieniu i używać jako kluczy w kontenerach asocjacyjnych oraz haszujących. Identyfikator jednoznacznie rozróżnia istniejące wątki programu, ale może zostać ponownie użyty po zakończeniu wątku.

```cpp
std::thread::id master_thread;

void some_core_part_of_algorithm()
{
    if (std::this_thread::get_id() == master_thread)
        do_master_thread_work();

    do_common_work();
}
```

Przed uruchomieniem wątków należy przypisać `master_thread` identyfikator wątku głównego; w przeciwnym razie będzie to domyślny identyfikator oznaczający brak wątku.
