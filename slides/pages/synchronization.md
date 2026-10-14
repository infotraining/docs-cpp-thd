---
transition: slide
layout: cover
background: /img/header-bg.svg
---

# Synchronizacja

---

## Poprawność programów współbieżnych

* Poprawny program współbieżny musi spełniać dwie właściwości:
  * Bezpieczeństwa - *Safety*
  * Żywotności - *Liveness*

---

## Bezpieczeństwo

* **Safety** - *nothing bad will ever happen*
  - nigdy nie doprowadza do **niepożądanego stanu**
  - bezpieczeństwo może być naruszone przez:
    - data-race - dwa wątki jednocześnie wchodzą do sekcji krytycznej
    - stan końcowy operacji nie spełnia postawionych wymagań (*invariant violation*)
    - deadlock

---

## Żywotność

* **Liveness** - *something good will eventually happen*
  - zapewnia, że każde pożądane zdarzenie w końcu zajdzie
  - opisuje **gwarancję postępu** programu
  - żywotność może być naruszona poprzez:
    - livelock
    - zagłodzenie wątków

---

## Thread-Safety = Safety ^ Liveness

* **Thread‑Safety** to własność obejmująca zarówno safety, jak i liveness.
* Segment kodu jest „thread-safe" jeśli:
  * manipuluje *współdzielonym stanem* w taki sposób, że gwarantowane jest bezpieczne (prawidłowe) wykonanie go przez wiele wątków pracujących w tym samym czasie
  * zapewnia *gwarancję postępu* - brak zagłodzenia wątków i livelock

---

## Unsafe code

* Nieprawidłowo chroni współdzielone dane oraz zasoby (data race)
* Zależy od przechowywanego między wywołaniami stanu - np. funkcja `strtok()`, `rand()`
* Nie gwarantuje postępu (może prowadzić do zagłodzenia wątków lub livelock)
* Wywołuje kod, który jest „unsafe"

---

## Jak osiągnąć Thread-Safety?

<v-clicks depth="2">

* Eliminacja współdzielonego stanu (najlepsza opcja)
  - niezmienność danych (**Immutability**)
  - **Message Passing** (np. kolejki komunikatów, kanały)
  - **Actor Model**
  - dane używane tylko przez jeden wątek - **Thread confinement**
* Synchronizacja dostępu do współdzielonego stanu
  * Stosowanie jawnego blokowania (**Explicit Locking**) 
    - muteksów i semaforów do synchronizacji dostępu do danych współdzielonych
    - blokad readers-writers (**Shared locks**)
  * Atomowość (**Atomicity**) 
    - stosowanie zmiennych i flag atomowych
    - struktury danych **Lock-free**
  
</v-clicks> 

---

## Jak osiągnąć Thread-Safety?

<v-clicks depth="2">

* Inwarianty i spójność danych
  * Thread‑safety nie polega tylko na „braku race condition”
  * należy zapewnić, że inwarianty są utrzymane w każdym interleavingu

* Unikanie deadlocków
  * stosowanie stałej kolejności blokowania zasobów
  * unikanie blokowania w callbackach
  * unikanie blokowania w destruktorach
  * unikanie blokowania w kodzie, który może być wywołany ponownie (reentrancy)

</v-clicks> 

---

## Jak osiągnąć Thread-Safety?

<v-clicks depth="2">

* Unikanie livelocków i starvation (liveness)
  * **fairness** w algorytmach *Lock‑free*
  * unikanie agresywnego spinowania
  * stosowanie backoff (np. exponential backoff)
  * priorytety wątków ustawione rozsądnie
  * unikanie „busy waiting” bez limitu

</v-clicks>

---

## Immutability

* Obiekty **immutable** są automatycznie **thread‑safe**.
* `const` w C++11 znaczy thread-safe
  * brak mutujących metod (enkapsulacja stałego stanu)
  * immutable views - `std::string_view` i `std::span<const T>`
  * `constexpr` - umożliwia tworzenie stałych wyrażeń, które są niezmienne i thread-safe

---

## Immutability - przykład

* Przykład klasy konfiguracyjnej, która jest niemutowalna i thread-safe

```c++
class AppConfig {
  const std::string name_;
  const size_t threads_count_;
  inline static constexpr size_t max_connections = 1024;
public:
  AppConfig(std::string name, size_t threads_count = std::thread::hardware_concurrency())
    : name_(std::move(name)), threads_count_(threads_count) {}

  std::string name() const { return name_; }
  size_t threads_count() const { return threads_count_; }
};
```

---

## Immutability - przykład

* Aby obiekt immutable gwarantował odpowiednio długi czas życia należy używać `std::shared_ptr<const T>`

```c++
using ImmutableAppConfig = std::shared_ptr<const AppConfig>;

void run_service(ImmutableAppConfig config) {
  // implementation of the service using the immutable configuration
}

void run(ImmutableAppConfig config) 
{
    std::jthread service_thread_1(&run_service, config);
    std::jthread service_thread_2(&run_service, config);

    // implementation of the main thread while the service threads are running
}

```

---

## Współdzielenie kontenerów standardowych

* Dozwolony jest współbieżny odczyt (operacje read-only) np:
  - wywołanie metod `begin()`, `end()`, `rbegin()`, `rend()`, `front()`, `back()`, `lower_bound()`, `upper_bound()`, `equal_range()`, `at()`
  - odczyt za pomocą `operator []` - z wyjątkiem kontenerów asocjacyjnych
  - dostęp za pomocą iteratorów, jeśli nie modyfikujemy stanu wskazywanych elementów
* Dozwolony jest współbieżny dostęp do różnych elementów kontenera za wyjątkiem obiektów `bitset` oraz `vector<bool>`

---

## Współdzielenie strumieni

* Dla formatowanych operacji wejścia oraz wyjścia na obiektach strumieni standardowych (`std::cin`, `std::cout`, `std::cerr`, …)
  - dozwolony jest dostęp współbieżny - brak *data race*
  - może wystąpić przemieszanie kolejności znaków (interleaved characters)
  - pojedyncze operacje wejścia/wyjścia są atomowe, ale sekwencje wielu operacji mogą być przerywane przez inne wątki (np. `std::cout << "Hello" << "World";`)

---
layout: center
---

<div class="slogan">
Sahred muteabl steta is na ivel of all multthrieading
</div>

---
layout: center
---

<div class="slogan">
Shared mutable state is an evil of all multithreading
</div>

---
layout: cover
background: /img/header-bg.svg
---

# Muteksy - Explicit Locking

---

## Sekcja krytyczna

* Fragment kodu programu, który w danej chwili może być wykonywany tylko przez jeden wątek
* Sekcja krytyczna zapewnia właściwość **wzajemnego wykluczenia**
* Powinna być **krótka i deterministyczna**, aby minimalizować czas blokady i ryzyko zakleszczenia (deadlock)

---

## Podstawowe obiekty blokad

* Muteksy (*mutual exclusion*) - obiekty implementujące właściwość wzajemnego wykluczania
* Semafory - binarne i zliczające
* Blokady współdzielone (blokady readers-writers)
* Zmienne warunkowe

---

## Muteks

* **Muteks** jest mechanizmem zapewniającym **mutual exclusion** — wzajemne wykluczanie
* Zapewnia wyłączny dostęp do zasobu
* W danym momencie **tylko jeden wątek może posiadać muteks**

---

## Muteks - operacje podstawowe

* Muteks ma dwa stany
  * zablokowany - **locked**
  * i odblokowany - **unlocked**
* Operacja`lock()` - blokuje muteks, jeśli jest dostępny, w przeciwnym razie wątek zostaje zablokowany do momentu jego odblokowania
* Operacja `unlock()` - odblokowuje muteks, umożliwiając innym wątkom jego pozyskanie
* Tylko wątek posiadający muteks może go odblokować poprzez wywołanie `unlock()`

---

## Muteksy w bibliotece standardowej

* Biblioteka standardowa C++ udostępnia kilka typów muteksów, w tym:
  * `std::mutex`
  * `std::recursive_mutex`
  * `std::timed_mutex`
  * `std::recursive_timed_mutex`
  * `std::shared_mutex` - C++17

---

## Koncepty obiektów blokad

* Standard definiuje następujące koncepty dla obiektów blokad:
  * `BasicLockable`
  * `Lockable`
  * `TimedLockable`
  * `SharedLockable`

---

## Koncept BasicLockable

```cpp
template<typename L>
concept BasicLockable =
requires (L m) {
    m.lock();
    m.unlock();
};
```

---

## Koncept Lockable

```cpp {all|4}
template<typename L>
concept Lockable = BasicLockable<L> &&
requires (L m) {
  { m.try_lock(); } -> std::convertible_to<bool>;
};
```

---

## std::mutex

* implementuje koncept `Lockable` zapewniając podstawowy mechanizm synchronizacji, który może być użyty do implementacji bezpiecznego dostępu do współdzielonego zasobu
* wywołujący wątek posiada muteks od momentu udanego wywołania metody `lock()` lub `try_lock()` do wywołania metody `unlock()`
* jeśli wątek posiada muteks, wszystkie inne wątki wywołujące `lock()` będą blokowane lub zwrócą false po wywołaniu `try_lock()`
* wywołujący wątek nie może posiadać muteksu przed wywołaniem `lock()` lub `try_lock()` - `std::mutex` implementuje nie-rekursywną wersję muteksu

---

## std::recursive_mutex

* implementuje rekursywną wersję konceptu `Lockable`
* ten sam wątek może wielokrotnie pozyskać muteks poprzez wywołanie metody `lock()` lub `try_lock()`
* aby zwolnić muteks wątek musi odpowiednią ilość razy wywołać `unlock()`
* maksymalny poziom rekursji dla obiektu `std::recursive_mutex` nie jest zdefiniowany przez standard, ale po przekroczeniu tej wartości:
  * z metody `lock()` rzucony zostanie wyjątek `std::system_error`
  * metoda `try_lock()` zwróci `false`

---

## Menadżery blokad

* Zarządzanie blokadami (obiektami muteksów) odbywa się za pomocą następujących klas implementujących RAII:
  * `std::lock_guard<Mutex>`
  * `std::unique_lock<Mutex>`
  * `std::shared_lock<Mutex>` - C++14 (w praktyce C++17)
  * `std::scoped_lock<Mutex>` - C++17

---

## std::lock_guard\<Mutex\>

<div class="text-code-08">
```cpp {all|7-8|13-14|10-11}
template<typename _Mutex>
class lock_guard
{
public:
    typedef _Mutex mutex_type;

    explicit lock_guard(mutex_type& __m) : _M_device(__m)
    { _M_device.lock(); }

    lock_guard(mutex_type& __m, adopt_lock_t) : _M_device(__m)
    { } // calling thread owns mutex

    ~lock_guard()
    { _M_device.unlock(); }

    lock_guard(const lock_guard&) = delete;
    lock_guard& operator=(const lock_guard&) = delete;

private:
    mutex_type&  _M_device;
};
```
</div>

---

## std::lock_guard\<std::mutex\> - przykład

<div class="text-code-07">

<<< @/../examples/06_mutex.cpp#lock_guard {all|1-6|8-13}

<<< @/../examples/06_mutex.cpp#lock_guard_usage {all}

</div>

---

## std::unique_lock

* Rozbudowana wersja managera blokad umożliwiająca:
  * ochronę RAII przed wyciekami blokad
  * opóźnione pozyskiwanie blokad - `deferred_lock`
  * adaptowanie pozyskanej przez wątek blokady - `adopt_lock`
  * transfer prawa własności 
    - instancja `unique_lock` nie jest kopiowalna, ale jest przenaszalna (moveable)
  * podejmowanie nieblokujących prób pozyskania blokady - `try_lock()`
  * korzystanie z muteksów czasowych

---

## std::unique_lock

* Jeśli instancja managera `std::unique_lock<>` posiada blokadę:
  * metoda `mutex()` zwraca wskaźnik do muteksu realizującego blokadę
  * metoda `owns_lock()` zwraca wartość `true`
  * w momencie niszczenia obiektu destruktor wywoła metodę `unlock()` na obiekcie muteksu

---

## std::unique_lock - std::try_to_lock

* Nieblokujące próby pozyskania blokady - `unique_lock(Lockable& m,std::try_to_lock_t)`
  * w konstruktorze wywoływana jest dla muteksu nieblokująca metoda `try_lock()`
  * jeśli blokada zostanie pozyskana metoda `owns_lock()` zwraca true, w przeciwnym wypadku zwraca `false`

---

## std::unique_lock + std::timed_mutex

<!-- old snippet
```cpp
std::timed_mutex mutex;
std::unique_lock<std::timed_mutex> lock(mutex, std::try_to_lock);

//...
if (!lock.owns_lock())
{
    int count = 0;
    do
    {
        std::cout << "Thread does not own a lock..."
                  << " Tries to acquire a mutex..."
                  << std::endl;
    } while(!lock.try_lock_for(std::chrono::seconds(1)));
}
```
-->

<<< @/../examples/06_mutex.cpp#timed_mutex

---
layout: cover
background: /img/header-bg.svg
---

# Deadlock

---
layout: center
---

## Deadlock

sytuacja, w której co najmniej dwa różne wątki czekają na siebie nawzajem, więc żadny nie może się zakończyć

---

## Deadlock - warunki

Do zakleszczenia dochodzi, jeśli spełnione są cztery warunki:

<v-clicks>

1. **Wzajemne wykluczenie** - w danym czasie tylko jeden wątek może korzystać z zasobu
2. **Trzymanie zasobu i oczekiwanie** - wątek utrzymuje jeden z zasobów, ale do ukończenia pracy potrzebne jest także zablokowanie innego zasobu
3. **Cykliczne oczekiwanie** - wątki w taki sposób żądają dostępu do zasobów, że powstaje cykliczny graf skierowany
4. **Brak wywłaszczania zasobu** - wątki dobrowolnie nie rezygnują z przydzielonych im zasobów, zwolnienie zasobów możliwe jest po zakończeniu zadania

</v-clicks>

---

## Deadlock - przykład

Klasa ze stanem wewnętrznym, chroniona muteksem. Chcemy napisać operator porównania.

<!-- old snippet
```cpp {all|8-10}
class X
{
    mutable std::mutex mtx_;
    int data_;
public:
    bool operator<(const X& other) const
    {
        std::lock_guard<std::mutex> lk(mtx_);
        std::lock_guard<std::mutex> lk(other.mtx_);
        return data_ < other.data_;
    }
};
```
-->

<div class="text-code-09">

<<< @/../examples/07_deadlock.cpp#deadlock {all|14-16}

</div>

---

## Deadlock

| Thread A                     | Thread B         |
|:-----------------------------|:-----------------|
| `if(x1 < x2)`                | `if(x2 < x1)`    |
|                              | `x2.mtx_.lock()` |
| `x1.mtx_.lock()`             |                  |
| `x2.mtx_.lock() <-` deadlock |                  |

---

## Zapobieganie zakleszczeniom - std::lock()

* Aby zminimalizować ryzyko należy pozyskiwać blokady zawsze w tej samej kolejności
* Lub użyć funkcji `std::lock()`
  * gwarantuje zablokowanie wszystkich muteksów bez zakleszczenia niezależnie od kolejności ich pozyskiwania
  * wymaga przekazania jako parametrów opóźnionych blokad typu `std::unique_lock`

---

## std::lock()

<!-- old snippet
```cpp {all|3-4|6|9}
bool X::operator< (const X& other) const
{
    std::unique_lock<std::mutex> l1(mtx_, std::defer_lock);
    std::unique_lock<std::mutex> l2(other.mtx_, std::defer_lock);

    std::lock(l1, l2); // avoiding deadlock

    return some_data < other.some_data;
}
```
-->

<div class="text-code-09">

<<< @/../examples/07_deadlock.cpp#std_lock {all|13-14|16-17}

</div>

---

## std::lock() - C++17

<!-- old snippet
```cpp {all|3-4|6|9}
bool X::operator< (const X& other) const
{
    std::unique_lock l1(mtx_, std::defer_lock);
    std::unique_lock l2(other.mtx_, std::defer_lock);

    std::lock(l1, l2); // avoiding deadlock

    return some_data < other.some_data;
}
```
-->

<div class="text-code-09">

<<< @/../examples/07_deadlock.cpp#std_lock_cpp17 {all|14-15|17-18|14-18}

</div>

---

## std::scoped_lock - C++17

<!-- old snippet
```cpp {all|3|6}
bool X::operator< (const X& other) const
{
    std::scoped_lock{mtx_, other.mtx_};
    
    return some_data < other.some_data;
}
```
-->

<div class="text-code-09">

<<< @/../examples/07_deadlock.cpp#scoped_lock {all|14-16}

</div>

--- 

## Blokady Readers/Writers

* Mechanizm synchronizacji, który pozwala wielu wątkom **czytać jednocześnie**, ale tylko jednemu **pisać**
* Implementacja w C++
  * `std::shared_mutex` lub `std::shared_timed_mutex`
  * `std::shared_lock` dla blokad do odczytu (readers)
  * `std::unique_lock` dla blokad do zapisu (writers)
* Kluczowy prymityw, gdy mamy dużo odczytów i mało zapisów
  * klasyczny przypadek w konfiguracjach, cache’ach, mapach DNS, strukturach indeksujących itd.

---

## Przykład użycia std::shared_mutex

<div class="text-code-08">

<<< @/../examples/06_mutex.cpp#shared_mutex_usage {all}

</div>

---
layout: cover
background: /img/header-bg.svg
---

# Synchronizacja za pomocą zdarzeń

---

## Zdarzenia

* W programach wielowątkowych często zachodzi potrzeba zsynchronizowania pracy wielu wykonywanych współbieżnie zadań
* Dzieje się tak w sytuacji, gdy jeden z pracujących wątków osiąga punkt, w którym nie może wykonać następnych operacji, dopóki inne wątki nie zakończą swojej części pracy, przygotowując dane potrzebne do zakończenia zadania
* Fakt, że oczekiwane dane są dostępne jest określany jako **zdarzenie** (*event*).

---

## Synchronizacja za pomocą zdarzeń

Oczekiwanie na zdarzenie może być zaimplementowane jako:

<v-clicks>

* **Busy wait** - oczekiwanie na zdarzenie implementowane jest za pomocą pętli `do-while`
* **Idle wait** - wątek oczekujący na zdarzenie przechodzi do stanu uśpienia, w którym nie zużywa cyklów CPU

</v-clicks>

---

## Busy waits - implementacja z muteksem

<!-- old snippet
```cpp
bool data_ready;
std::mutex mtx;
```

<div class="grid grid-cols-2 gap-4">
<div>

```cpp
void set_data_ready()
{
  prepare_data();
  std::unique_lock<std::mutex> lk(mtx);
  data_ready = true;
}
```

</div>
<div>

```cpp
void task()
{
  bool ready_flag = data_ready;

  do
  {
    {
      std::lock_guard<std::mutex> lk{mtx};
      ready_flag = data_ready;
    }
  } while(!ready_flag);

  process_data();
}
```

</div>
</div>
-->

<<< @/../examples/08_events.cpp#busy_mutex_decl

<div class="grid grid-cols-2 gap-4">
<div>

<<< @/../examples/08_events.cpp#busy_mutex_set {all|3|4-5}

</div>
<div>

<<< @/../examples/08_events.cpp#busy_mutex_task {all|3-9|11}

</div>
</div>

---

## Busy waits - implementacja z std::atomic\<bool\>

<!-- old snippet
```cpp
std::atomic<bool> data_ready;
```

<div class="grid grid-cols-2 gap-4">
<div>

```cpp
void set_data_ready()
{
    prepare_data();
    data_ready = true;
}
```

</div>
<div>

```cpp
void task()
{
    while(!data_ready)
    {
        //..
        std::this_thread::yield();
    }

    process_data();
}
```

</div>
</div>
-->

<<< @/../examples/08_events.cpp#busy_atomic_decl

<div class="grid grid-cols-2 gap-4">
<div>

<<< @/../examples/08_events.cpp#busy_atomic_set {all|3-4}

</div>
<div>

<<< @/../examples/08_events.cpp#busy_atomic_task {all|3-7|9 }

</div>
</div>

---

## Busy waits - z memory_order

<!-- old snippet
```cpp
std::atomic<bool> data_ready;
```

<div class="grid grid-cols-2 gap-4">
<div>

```cpp {all|4-6}
void set_data_ready()
{
    prepare_data();
    data_ready.store(true, 
      std::memory_order_release);
}
```

</div>
<div>

```cpp {all|3-4}
void task()
{
    while(!data_ready.load(
        std::memory_order_acquire))
    {
        //..
        std::this_thread::yield();
    }

    process_data();
}
```

</div>
</div>
-->

<<< @/../examples/08_events.cpp#busy_atomic_decl

<div class="grid grid-cols-2 gap-4">
<div class="text-09">

<<< @/../examples/08_events.cpp#busy_mo_set {all|4-5}

</div>
<div class="text-09">

<<< @/../examples/08_events.cpp#busy_mo_task {all|3}

</div>
</div>

---

## Idle waits - zmienne warunkowe

* Aby uniknąć aktywnego odpytywania się o spełnienie określonego warunku, efektywniej jest zablokować oczekujący wątek do momentu zajścia zdarzenia (spełnienia oczekiwanego warunku)
* Mechanizm taki jest wykorzystany w implementacji **zmiennych warunkowych** (*condition variables*)

---

## Zmienne warunkowe

* Biblioteka standardowa C++11 dostarcza dwie implementacje zmiennych warunkowych:
  - `std::condition_variable`
  - `std::condition_variable_any`

* Obie klasy współpracują z muteksem, aby zapewnić prawidłową synchronizację wątków:
  - `conditional_variable` współpracuje tylko z typem `std::mutex`
  - `conditional_variable_any` współpracuje z dowolnym typem muteksu

---

## Idle waits - CV

<!-- old snippet
```cpp
bool data_ready = false;
std::mutex mtx_ready;
std::condition_variable cv_ready;
```

<div class="grid grid-cols-2 gap-4">
<div>

```cpp
void set_data_ready()
{
  prepare_data();

  {
    std::lock_guard<std::mutex> lk(mtx_ready);
    data_ready = true;
  } // release lock

  cv_ready.notify_one();
}
```

</div>
<div>

```cpp
void task()
{
  std::unique_lock<std::mutex> lk(mtx_ready);
  while(!data_ready)
  {
    cv_ready.wait(lk);
  }
  lk.unlock();

  process_data();
}
```

</div>
</div>
-->

<<< @/../examples/08_events.cpp#cv_decl

<div class="grid grid-cols-2 gap-4">
<div>

<<< @/../examples/08_events.cpp#cv_set {all|3|5-8|10}

</div>
<div>

<<< @/../examples/08_events.cpp#cv_task {all|3|4-6|4,6-8|10}

</div>
</div>

---

## Spourious weakups

* Wątki oczekujące na zmienne warunkowe mogą zostać obudzone bez spełnienia oczekiwanego warunku
* Dlatego <span v-mark.underline.orange>zawsze należy sprawdzać warunek w pętli po powrocie z `wait()`</span>
* Takie nieoczekiwane przebudzenia nazywane są **spurious wakeups**
* Powody wybudzeń:
  * optymalizacje jądra systemu operacyjnego (kernel może obudzić wątek "przy okazji" migracji CPU)
  * sygnały wysyłane przez inne wątki lub procesy
  * implementacja futexów w jądrze systemu operacyjnego

---

## Idle waits - CV (z predykatem)

<!-- old snippet
```cpp
bool data_ready = false;
std::mutex mtx_ready;
std::condition_variable cv_ready;
```

<div class="grid grid-cols-2 gap-4">
<div>

```cpp
void set_data_ready()
{
  prepare_data();

  {
    std::lock_guard<std::mutex> lk(mtx_ready);
    data_ready = true;
  } // release lock

  cv_ready.notify_one();
}
```

</div>
<div>

```cpp {all|4}
void task()
{
  std::unique_lock<std::mutex> lk(mtx_ready);
  cv_ready.wait(lk, [] { return data_ready; });

  lk.unlock();

  process_data();
}
```

</div>
</div>
-->

<<< @/../examples/08_events.cpp#cv_decl

<div class="grid grid-cols-2 gap-4">
<div>

<<< @/../examples/08_events.cpp#cv_set {all}

</div>
<div>

<<< @/../examples/08_events.cpp#cv_pred_task {all|3-5|7-9}

</div>
</div>

---

## Powiadamianie o zdarzeniu

* Powiadomienie o zdarzeniu może zostać zrealizowane przy pomocy dwóch metod:
  - `void notify_one()` – odblokowuje jeden z wątków znajdujących się w stanie oczekiwania po uprzednim wywołaniu na obiekcie zmiennej warunkowej metody `wait()`
  - `void notify_all()` – odblokowuje wszystkie wątki znajdujące się w stanie oczekiwania

---

## Idle waits - std::atomic - C++20

<!-- old snippet
```cpp
std::atomic<bool> data_ready = false;
```

<div class="grid grid-cols-2 gap-4">
<div>

```cpp {all|5-8}
void set_data_ready()
{
  prepare_data();

  data_ready = true;
  data_ready.notify_one();
}
```

</div>
<div>

```cpp {all|3}
void task()
{
  data_ready.wait(false);

  process_data();
}
```

</div>
</div>
-->

<<< @/../examples/08_events.cpp#atomic_wait_decl

<div class="grid grid-cols-2 gap-4">
<div>

<<< @/../examples/08_events.cpp#atomic_wait_set {all|5-6}

</div>
<div>

<<< @/../examples/08_events.cpp#atomic_wait_task {all|3}

</div>
</div>

---
layout: cover
background: /img/header-bg.svg
---

# Lokalna pamięć wątku

---

## Lokalna pamięć wątku

* *Thread Local Storage* - dane, które należą do konkretnego wątku
* Każdy wątek może posiadać własny zestaw zmiennych TLS
* Odpowiednik statycznych (globalnych) zmiennych dla wątków
* Zastosowanie:
  * `errno`
  * funkcje rentrant
    * generatory liczb losowych 
    * `strtok()`

---

## Modyfikator thread_local

C++11 zapewnia dostęp do danych TLS za pomocą modyfikatora `thread_local`

---

## Unsafe random

<!-- old snippet
```cpp
namespace Unsafe {
    double rand() {
        static int seed = 665; // shared state

        seed = (seed * IMUL + IADD) & MASK;
        return (seed * SCALE);
    }
}

int main() {
    std::thread thd1([] {
        for (size_t i = 0; i < 100; ++i)
            std::cout << Unsafe::rand() << std::endl;
    });

    std::thread thd2([] {
        for (size_t i = 0; i < 100; ++i)
            std::cout << Unsafe::rand() << std::endl;
    });

    thd1.join();
    thd2.join();
}
```
-->

<div class="text-code-sm">

<<< @/../examples/09_tls.cpp#unsafe_random {all}

<<< @/../examples/09_tls.cpp#unsafe_random_usage {all}

</div>

---

## ThreadSafe random

<!-- old snippet
```cpp {all|3-11|6}
namespace ThreadSafe {
    double rand_double() {
        std::hash<std::thread::id> hasher;
        thread_local int thd_seed = 665 * hasher(std::this_thread::get_id());

        int result = (thd_seed * IMUL + IADD) & MASK;
        thd_seed = result;
        return (result * SCALE);
    }
}

int main() {
    std::thread thd1([] {
        for (size_t i = 0; i < 100; ++i)
            std::cout << ThreadSafe::rand_double() << std::endl;
    });

    std::thread thd2([] {
        for (size_t i = 0; i < 100; ++i)
            std::cout << ThreadSafe::rand_double() << std::endl;
    });

    thd1.join();
    thd2.join();
}
```
-->

<div class="text-code-sm">

<<< @/../examples/09_tls.cpp#thread_safe_random {all|3-10|6-7}

<<< @/../examples/09_tls.cpp#thread_safe_random_usage {all}

</div>

---
layout: cover
background: /img/header-bg.svg
---

# Synchronizacja w C++20

---

## Nowe mechanizmy synchronizacji w C++20

<v-clicks>

* `std::latch`
  * jednorazowy atomowy licznik, który blokuje do momentu osiągnięcia wartości `0`
* Bariery
  * liczniki wielokrotnego użytku
* Semafory
  * lekkie obiekty synchronizujące
  * umożliwiają ograniczanie dostępu do zasobów

</v-clicks>

---

## std::latch

Przydatny w testach, gdy chcemy mieć pewność, że wszystkie wątki wystartowały

<!-- old snippet
```cpp {all|1-2|4|13}
void background_work(const int id, const std::string text, 
                     std::latch& all_ready, std::latch& all_done) 
{
    all_ready.arrive_and_wait();
    
    std::cout << "BW#" << id << " started..." << std::endl;

    for (const auto& letter : text) {
        std::cout << "Thread#" << id << " - " << letter << std::endl;
        std::this_thread::sleep_for(25ms);
    }

    all_done.count_down();
}
```
-->

<<< @/../examples/10_cpp20_sync.cpp#latch_task {all|1-2|4|14}

---

## std::latch - użycie

<!-- old snippet
```cpp {all|1-3|5-7|9}
std::latch all_ready(4);
std::latch all_done(4);
std::vector<std::jthread> thds;

for(size_t i = 0; i < 4; ++i)
    thds.emplace_back([id = i + 1, &all_ready, &all_done]{ 
        background_work(id, "LATCH", all_ready, all_done); });

all_done.wait();
```
-->

<<< @/../examples/10_cpp20_sync.cpp#latch_usage {all|3-5|7-10|12}

---

## std::barrier

* Atomowy licznik wielokrotnego użytku (w odróżnieniu od `std::latch`)
* Obiekt bariery przechodzi w trakcie swojego życia kilka faz
* Każda faza definiuje tzw. *phase synchronization point* - punkt, w którym czekające wątki są blokowane

---

## std::barrier - API

* Wątek może dojść do bariery i czekać aż licznik osiągnie `0`: 

```cpp
barrier.arrive_and_wait()
```

* lub opóźnić czekanie: 
  
```cpp
auto arrival_token = barrier.arrive(); 
//...
barrier.wait(std::move(arrival_token));
```

---

## std::barrier - fazy

Faza bariery składa się z następujących kroków:

1. Licznik jest dekrementowany przez wywołanie `arrive()` lub `arrive_and_drop()`
2. Kiedy wartość licznika osiąga zero, wywoływana jest tzw. *phase completion step*
   - uruchamiana jest funkcja `CompletionFunction`
   - wraz z zakończeniem funkcji `CompletionFunction` odblokowywane są wszystkie czekające wątki
3. Wraz z końcem *phase completion step* licznik jest resetowany do wartości ustawionej w konstruktorze bariery, pomniejszonej o ilość wywołań `arrive_and_drop()`

---

## std::barrier - przykład

<!-- old snippet
```cpp {all|5-12|12-20}
const auto workers = { "A", "B", "C" };
static int counter = 0;
static std::string phase = "";

auto on_completion = []() noexcept {
    // locking is not needed here
    ++counter;
    phase = "--- done no. "s + std::to_string(counter) + "\n";
    std::cout << phase;
};

std::barrier sync_point(std::ssize(workers), on_completion);

auto work = [&sync_point](std::string name) {
    for(int i = 0; i < 4; ++i) {
        std::string product = ".." + name + " worked\n";
        std::cout << product;
        sync_point.arrive_and_wait();
    }
}; 

std::vector<std::jthread> thds;
for(const auto& worker : workers) 
    thds.emplace_back(work, worker);
```
-->

<div class="text-code-07">

<<< @/../examples/10_cpp20_sync.cpp#barrier {all|6-11|13|15-22}

</div>

---

## Nowe typy atomowe

* Atomowa referencja `std::atomic_ref<T>`
  * `std::atomic<>` dla referencji
  * specjalizacja dla *raw-pointers*
* `std::atomic<std::shared_ptr<T>>`
  * zapewnia atomowe operacje dla wskaźników współdzielonych
* Specjalizacja `std::atomic<double>` i `std::atomic<float>`
* Mechanizm synchronizacji eventowej dla typów atomowych

---

## std::atomic_ref

<!-- old snippet
```cpp {all|3,14-19}
const int thds_count = 10;
std::latch all_ready{thds_count};
std::vector<uint32_t> data(10);

{
    std::vector<std::jthread> thds;

    for(int i = 0; i < thds_count; ++i)
        thds.emplace_back([&] {
            all_ready.arrive_and_wait();

            for(int i = 0; i < 100'000; ++i)
            {
                for(auto& item : data)
                {
                    //++item; // BUG!!! -> data-race
                    std::atomic_ref synced_value{item}; // data-race free access
                    ++synced_value;
                }
            }
        });
}
```
-->

<div class="text-code-07">

<<< @/../examples/10_cpp20_sync.cpp#atomic_ref {all|5|16-21}

</div>

---

## Synchronizowane strumienie wyjściowe

<!-- old snippet
```cpp {all|5,9-12}
#include <syncstream>

void do_calculations(int n)
{
    std::osyncstream synced_cout{std::cout};

    for(int i = 0; i < n; ++i)
    {
        synced_cout 
            << "Calculated value is " 
            << fibonacci(i) << "\n"
            << std::flush_emit;
    }
}

std::jthread thd_1{do_calculations, 100};
std::jthread thd_2{do_calculations, 100};
```
-->

<<< @/../examples/10_cpp20_sync.cpp#osyncstream {all|3|7-10}
