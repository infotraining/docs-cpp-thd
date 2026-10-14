---
layout: cover
background: /img/header-bg.svg
---

# std::future

## High-Level API for concurrency

---

## std::future\<T\>

obsługuje możliwość odczytania wyniku wykonania funkcji (zwracanej wartości typu `T` lub wyjątku), która została uruchomiona w tym samym lub innym wątku

---

## Stan współdzielony - shared state

obiekt synchronizujący, który przechowuje wynik (prawdopodobnie jeszcze nie obliczony), który może być wartością lub wyjątkiem

---

## Shared state

jest współdzielony między:

* *asynchronous return object* - blokuje wykonanie programu, dopóki wynik nie jest gotowy - `std::future`
* *asynchronous provider* - zapisuje wynik w obiekcie shared-state

---
class: white-slide
---

## Shared state

<img src="/img/shared-state-1.png" class="h-80 mx-auto"/>

---
class: white-slide
---

## Shared state

<img src="/img/shared-state-2.png" class="h-80 mx-auto"/>

---
class: white-slide
---

## Shared state

<img src="/img/shared-state-3.png" class="h-80 mx-auto"/>

---

## std::future

* Do obsługi wartości typu `std::future` wykorzystywane są cztery klasy:
  * `std::future<T>`
  * `std::shared_future<T>`
  * `std::promise<T>`
  * `std::packaged_task<T>`
  * oraz funkcja `std::async()`

---

## std::future\<T\> - interfejs klasy

* `T get()`
  - wstrzymuje bieżący wątek do momentu zakończenia asynchronicznej funkcji i następnie zwraca otrzymaną wartość lub rzuca przechowany w shared state wyjątek
* `void wait() const`
  - blokuje bieżący wątek dopóki wynik nie zostanie obliczony
* `future_status wait_for(const chrono::duration& timeout) const` 
  * czeka przez określony interwał czasu aż wynik zostanie obliczony. Zwracany jest status, który może przyjąć jedną z trzech wartości:
    - `future_status::deferred`
    - `future_status::timeout`
    - `future_status::ready`

---
class: white-slide
---

## std::future

<img src="/img/async.png" class="h-80 mx-auto"/>

---

## std::future vs. std::shared_future

`std::future<T>` - wynik przyszłego wywołania funkcji

`std::shared_future<T>` - implementacja współdzielonych wartości typu future

---

## Wywołania asynchroniczne funkcji - std::async

* `std::async(std::launch policy, Function&& f, Args&&... args)`
  * wywołuje funkcję `f` przekazując do niej argumenty `args`
  * zwraca `std::future<std::result_of_t<Function(Args...)>>`

---

## Wywołania asynchroniczne funkcji - std::async

Jako zadanie do funkcji `std::async` można przekazać:
* wskaźnik do funkcji
* obiekt funkcyjny
* obiekt domknięcia
* obiekt + wskaźnik do metody

---

## Wywołania asynchroniczne funkcji - std::async

<!-- old snippet
```cpp
int fib(int n);
Gadget g{42, "ipad"};

std::future<int> f1 = std::async(std::launch::async, fib, 10);
std::future<int> f2 = std::async(std::launch::async, [] { return fib(20);});
std::future<int> f3 = std::async(std::launch::async, [g] { g.use(); });
std::future<int> f4 = std::async(std::launch::async, &Gadget::use, g);
```
-->

* Przykłady wywołań asynchronicznych funkcji przy użyciu `std::async`:

<<< @/../examples/11_futures.cpp#async_callables

---

## std::async - argumenty funkcji

* Argumenty funkcji, które są
  * *lvalue* - są kopiowane (również wtedy, gdy są to referencje)
  * *rvalue* - są przenoszone

* Aby przekazać parametry przez referencję należy:
  * `std::ref` - przez referencję
  * `std::cref` - przez referencję do stałej

* Argumenty mogą też zostać przechwycone do obiektu domknięcia

---

## std::async - argumenty funkcji

<!-- old snippet
```cpp
void work(int id, const std::string& path, std::atomic<int>& counter);

int id = 1;
const std::string path = "/dev/home/";
std::atomic<int> counter{0};

{
    using namespace std;

    auto f1 = async(launch::async, 
                    &work, id, cref(path), ref(counter));
    
    auto f2 = async(launch::async,
                    [id, &path, &counter] { work(id, path, counter); });                    
}
```
-->

<<< @/../examples/11_futures.cpp#async_args

---

## Wyliczenie std::launch::async

specyfikuje sposób uruchomienia funkcji przekazanej do `std::async()`

```cpp
enum class std::launch 
{
  async, 
  deferred
};
```

---

## Wyliczenie std::launch::async

* `std::launch::async` 
  - funkcja jest uruchamiana w osobnym wątku (asynchronicznie)
* `std::launch::deferred` 
  - nie tworzy osobnego wątku. Leniwie wykonuje funkcję f (wywołanie następuje w momencie pierwszego wywołania na obiekcie future `get()` lub `wait()`

---

## std::async - destruktory obiektów std::future

jeśli obiekt typu `std::future` został utworzony przez wywołanie `std::async`, to jego destruktor **blokuje** wykonanie wątku, do czasu zakończenia wykonania funkcji asynchronicznej!

---

## std::async - destruktory - BUG in std!!!

<!-- old snippet
```cpp
void save_to_file(const std::string& path, const std::string& msg);

{
  std::async(std::launch::async, &save_to_file, "a.log", "Do not do");
  std::async(std::launch::async, &save_to_file, "b.log", "like THIS");
}
```
-->

<<< @/../examples/11_futures.cpp#async_destructor_bad

---

## std::async - destruktory - poprawna wersja

<!-- old snippet
```cpp {all|4,5}
void save_to_file(const std::string& path, const std::string& msg);

{
  auto f1 = std::async(std::launch::async, &save_to_file, "a.log", "Do LIKE");
  auto f2 = std::async(std::launch::async, &save_to_file, "b.log", "THIS");
}
```
-->

<<< @/../examples/11_futures.cpp#async_destructor_good {all|3-4}

Przypisanie do nazwanych zmiennych powoduje, że funkcje są rzeczywiście wykonywane asynchronicznie

---

## std::packaged_task

jest klasą dostarczającą wrapper umożliwiający wywołanie funkcji lub funktora i zapisujący wynik w *shared state*, który może być odczytany przez obiekt `std::future`

---

## std::packaged_task

* pozwala oddzielić:
  * definicję zadania, które ma być wykonane
  * wywołanie zadania, które może zostać wykonane w osobnym wątku
  * przetworzenie wyników zadania

---

## std::packaged_task

<!-- old snippet
```cpp {all|3|5|7-8|12}
int calculate(int arg); // may throw std::runtime_error

std::packaged_task<int()> ptask1([] { return calculate(665); });

std::future<int> f1 = ptask1.get_future();

std::thread thd1(std::move(ptask1));
thd1.detach();

try 
{
    auto result = f1.get();
} 
catch(const std::runtime_error& e) 
{
   // handle the exception
}
```
-->

<<< @/../examples/11_futures.cpp#packaged_task {all|3|5|7|9-17}

---

## std::promise

* asynchronous provider
* implementuje niskopoziomowy mechanizm przekazania asynchronicznego wyniku do obiektu `std::future`, wykorzystując *shared state*
* wykorzystywany w sytuacji, gdy kod obliczający wartość nie może być zapakowany do `std::packaged_task` lub przekazany do `std::async`

---

## std::promise - API

* konstruktor - tworzy pusty obiekt *shared state*
* `set_value(T)` - zapisuje wartość do stanu współdzielonego
* `set_exception(T)` - zapisuje wyjątek do stanu współdzielonego
* destruktor
  * jeśli stan współdzielony jest gotowy, zwalnia go
  * jeśli stan współdzielony nie jest gotowy, ustawia jako wyjątek instancję typu `std::future_error` z kodem błędu `std::future_error::broken_promise` i zwalnia go

---

## std::promise - kod

<!-- old snippet
```cpp {all|2|8-10|15-18|19-22}
class DataSource {
    std::vector<std::promise<Data>> channels_;

public:
    DataSource(size_t channels_count) : channels_(channels_count)
    {}

    std::future<Data> get_future_channel(size_t channel_no) {
        return channels_.at(channel_no).get_future();
    }

    void read_data() {
        for(size_t i = 0; i < channels_.size(); ++i) {
            std::this_thread::sleep_for(250ms);
            try {
                auto result = calculate(i);
                channels_[i].set_value(result);       
            }
            catch(...) {
                std::exception_ptr eptr = std::current_exception();
                channels_[i].set_exception(eptr);
            }            
        }
    }
};
```
-->

<div class="text-code-07">

<<< @/../examples/11_futures.cpp#promise_data_source {all|2|9-11|16-19|20-23}

</div>

---

## std::promise - użycie

<!-- old snippet
```cpp {all|2|4,5|7-8|10,11}
const size_t channel_count = 128;
DataSource data_source(channel_count);

std::future<int> future_channel_4 = data_source.get_future_channel(4);
std::future<int> future_channel_13 = data_source.get_future_channel(13);

std::thread thd{[ds = std::move(data_source)]() mutable { ds.read_data(); }};
thd.detach();

assert(future_channel_4.get() == 16);
assert_throws(future_channel_13.get();
```
-->

<div class="text-code-08">

<<< @/../examples/11_futures.cpp#promise_usage {all|3-4|6-7|9|11-21}

</div>