# Synchronizacja wątków

```{epigraph}
The problem with shared mutable state is that when you share, you mutate, and when you mutate, you need locks, and when you have locks, you get problems.

-- Joe Armstrong (creator of Erlang)
```

## Poprawność programów współbieżnych

**Poprawność współbieżna** (*Concurrent Correctness*) odnosi się do właściwości i atrybutów, które zapewniają, że system współbieżny (system, w którym wiele procesów lub wątków jest wykonywanych jednocześnie) działa zgodnie z oczekiwaniami i produkuje poprawne wyniki.

Właściwości związane z poprawnością programu współbieżnego:

1. **Bezpieczeństwo** (*Safety*) - program współbieżny jest bezpieczny, jeśli nigdy nie doprowadza do niepożądanego stanu.
  
   * **Wzajemne wykluczanie** (*Mutual Exclusion*) - zapewnia, że sekcje krytyczne kodu nie są wykonywane jednocześnie przez więcej niż jeden wątek lub proces.
  
   * **Race Condition** - sytuacja, w której wynik programu zależy od kolejności wykonania instrukcji przez wątki.

2. **Żywotność** (*Liveness*) - program współbieżny jest żywotny, jeśli zapewnia, że każde pożądane zdarzenie w końcu zajdzie.

   * **Deadlock-Free** - zapewnia, że nie ma sytuacji, w której dwa lub więcej wątków czeka na siebie nawzajem na zwolnienie zasobów, powodując zablokowanie systemu.
  
   * **Livelock-Free** - zapewnia, że wątki nie zmieniają stale stanów bez robienia żadnych postępów.

   * **Starvation-Free** - zapewnia, że wszystkie wątki lub procesy mają szansę na wykonanie i nie są nieskończenie długo pozbawione zasobów.

3. **Sprawiedliwość** (*Fairness*) - zapewnia, że wszystkie procesy lub wątki są traktowane jednakowo i mają równy dostęp do zasobów, zapobiegając scenariuszom, w których niektóre procesy otrzymują więcej zasobów niż inne.

4. **Deterministyczność** (*Determinism*) - program współbieżny jest deterministyczny, jeśli zawsze zwraca ten sam wynik dla tych samych danych wejściowych, niezależnie od tego, w jakiej kolejności wątki są wykonane.

5. **Spójność** (*Consistency*) - zapewnia, że wyniki operacji wykonywanych przez wątki są zgodne z oczekiwaniami, nawet w obliczu współbieżności.

6. **Atomowość** (*Atomicity*) - zapewnia, że operacje lub transakcje są zakończone jako pojedyncza, niepodzielna jednostka, zapobiegając częściowym aktualizacjom lub niespójnym stanom.

7. **Izolacja** (*Isolation*) - zapewnia, że operacje współbieżne są izolowane od siebie, zapobiegając interferencji i zapewniając, że każda operacja wydaje się być wykonana w izolacji.

Niepoprawnie zaprojektowane zależności między wątkami często naruszają integralność i żywotność danych.

### Thread-safety

Bezpieczeństwo wątków w programowaniu współbieżnym (*thread-safety*) jest pojęciem stosowanym w kontekście programów wielowątkowych.

Segment kodu jest **"thread-safe"** jeśli manipuluje współdzielonym stanem w taki sposób, że gwarantowane jest bezpieczne (prawidłowe) wykonanie go przez wiele wątków pracujących w tym samym czasie.
Oznacza to, że kod "thread-safe" musi w odpowiedni sposób chronić współdzielony między wątkami stan programu.

#### Kod unsafe

Kod "unsafe" to kod, który:

* nieprawidłowo chroni współdzielone dane oraz zasoby (*shared state*)
* zależy od przechowywanego między wywołaniami stanu (np. funkcja ``strtok()``, ``rand()``)
* wywołuje kod, który jest "unsafe"

#### Kod thread-safe

Kod "thread-safe" to kod, który zapewnia poprawność współbieżną. 

Bezpieczeństwo wątków można osiągnąć za pomocą różnych technik, takich jak:

* **Niezmienność danych** (*Immutability*) - stan obiektu nie może być zmieniony po jego utworzeniu. Obiekty mogą być bezpiecznie współdzielone, jeśli wątki mogą jedynie odczytywać ich stan.
* **Jawne blokowanie** (*Explicit Locking*) - stosowanie muteksów lub semaforów do ochrony danych współdzielonych. Operacje synchronizacji mogą znacznie spowolnić działanie programu.
* **Atomowość** (*Atomicity*) - stosowanie zmiennych i flag atomowych

## Współdzielenie danych

```{epigraph}
Shared mutable state is the root of all evil.

-- Edgar Dijkstra
```

### const - immutable state

* Zmienne, które są niezmienne (*immutable*) mogą być bezpiecznie współdzielone bez blokad
* Modyfikator `const` oznacza od C++11 *thread-safe*

```{attention}
Współbieżny dostęp do zmiennych, w czasie gdy przynajmniej jeden wątek modyfikuje stan współdzielonego obiektu (**data race**) może skutkować **niezdefiniowanym zachowaniem** (*undefined behaviour*).
```

### Współdzielenie obiektów biblioteki standardowej

#### Kontenery standardowe

Dla kontenerów STL oraz adapterów kontenerów:

* Dozwolony jest współbieżny odczyt (operacje *read-only*) np:
  * wywołanie metod ``begin()``, ``end()``, ``rbegin()``, ``rend()``, ``front()``, ``back()``, ``lower_bound()``
    ``upper_bound()``, ``equal_range()``, ``at()``
  * ``operator []`` - z wyjątkiem kontenerów asocjacyjnych
  * dostęp za pomocą iteratorów, jeśli nie modyfikujemy stanu wskazywanych elementów

* Dozwolony jest współbieżny dostęp do różnych elementów kontenera

  * za wyjątkiem obiektów ``bitset`` oraz ``vector<bool>``

#### Strumienie

Dla formatowanych operacji wejścia oraz wyjścia na obiektach strumieni standardowych (``cin``, ``cout``, ``cerr``, ...)

* Dozwolony jest dostęp współbieżny (z wielu wątków)
* Może wystąpić przemieszanie kolejności znaków (*interleaved characters*) - jeśli wiele wątków zapisuje do jednego strumienia `std::cout` jednocześnie, wynik może być nieprzewidywalny
* Standardowe strumienie są buforowane - użycie `std::flush` oraz `std::endl` wymusza opóżnienie buforów i zapis do strumienia

##### Synchronizowane strumienie - C++20

Od C++20 można użyć strumieni synchronizowanych, które zapewniają bezpieczny dostęp do strumieni standardowych z wielu wątków. 

Klasa `std::osyncstream` zapewnia synchronizację dostępu do strumienia.

Może być używana jako nazwana zmienna - zapisy są buforowane i opróżniane w momencie wywołania destruktora lub metody `emit()`.

```cpp
{
    std::osyncstream safe_out(std::cout); // synchronized wrapper for std::cout
    safe_out << "Hello, ";
    safe_out << "World!";
    safe_out << std::endl; // flush is noted, but not yet performed
    safe_out << "and more!\n";
} // characters are transferred and std::cout is flushed
```

lub jako tymczasowy obiekt opakowujący i synchronizujący strumień:

```cpp
std::osyncstream(std::cout) << "Hello, " << "World!" << '\n';
```

## Jawne wykluczanie (*Explicit Locking*)

### Sekcja krytyczna

Fragment kodu programu, który w danej chwili może być wykonywany tylko przez jeden wątek. Sekcja krytyczna zapewnia
właściwość wzajemnego wykluczenia.

Standardową realizacją wzajemnego wykluczenia jest wykorzystanie obiekt blokady (muteksu) zawierającego operacje:

* `lock()`
* `unlock()`

Wątek

* pozyskuje blokadę (blokuje), kiedy wywołuję metodę `lock()`
* zwalnia blokadę (odblokowuje), gdy wywołuje metodę `unlock()`

Wątek jest prawidłowo skonstruowany, jeżeli spełnia poniższe warunki:

* Każda sekcja krytyczna jest skojarzona z niepowtarzalnym obiektem blokady
* Wątek, próbując wejść do sekcji krytycznej, wywołuje metodę `lock()` tego obiektu
* Wątek wychodząc z sekcji krytycznej, wywołuje metodę `unlock()`

### Podstawowe obiekty blokad

* **Muteksy** (*Mutual Exclusion*) - obiekty implementujące właściwość wzajemnego wykluczania
* **Semafory** - binarne i zliczające
* **Blokady współdzielone** (blokady *readers-writers*)

### Zmienne warunkowe - warunkowe wzajemne wykluczenie

* Zmienne warunkowe używane są do powiadamiania wątków o zaistnieniu określonego warunku
* Są powiązane z muteksem, który jest ponownie pozyskiwany w momencie wybudzenia z blokady
* Warunek wybudzenia z blokady jest zdefiniowany przy pomocy predykatu, który jest parametrem blokady

## Muteksy i klasy menadżerów blokad

Muteksy zaimplementowane w bibliotece standardowej C++ korzystają z następujących konceptów obiektów blokad (*lockable objects*):

* BasicLockable
* Lockable
* TimedLockable
* SharedLockable

### Koncepty BasicLockable, Lockable i muteksy

Koncept `BasicLockable` modeluje właściwość wzajemnego wykluczenia. Typ ``L`` jest zgodny z wymaganiami
konceptu `BasicLockable` gdy w interfejsie posiada metody:

* ``void lock()`` - bieżący wątek jest wstrzymany, aż do momentu pozyskania blokady
* ``void unlock()`` - jeżeli bieżący wątek jest posiadaczem blokady, to następuje jej zwolnienie

Koncept `Lockable` rozszerza koncept `BasicLockable` o wymóg posiadania metody:

* ``bool try_lock()`` - próba pozyskania blokady bez wstrzymywania bieżącego wątku. Zwraca ``true`` jeśli blokada została pozyskana, w przeciwnym wypadku zwraca `false`.

#### `std::mutex`

Klasa `std::mutex` implementuje koncept *Lockable* zapewniając podstawowy mechanizm synchronizacji, który może być użyty do implementacji
bezpiecznego dostępu do współdzielonego zasobu.

* Wywołujący wątek posiada muteks od momentu udanego wywołania metody `lock()` lub `try_lock()` do wywołanie metody `unlock()`.
* Jeśli wątek posiada muteks, wszystkie inne wątki wywołujące `lock()` będą blokowane lub zwrócą `false` po wywołaniu `try_lock()`
* Wywołujący wątek nie może posiadać muteksu przed wywołaniem  `lock()` lub `try_lock()` - `std::mutex` implementuje nie-rekursywną wersję muteksu.

#### `std::recursive_mutex`

Klasa `std::recursive_mutex` implementuje rekursywną wersję konceptu `Lockable`. Ten sam wątek może wielokrotnie pozyskać muteks poprzez wywołanie
metody `lock()` lub `try_lock()`. Aby zwolnić muteks wątek musi odpowiednią ilość razy wywołać `unlock()`.

Maksymalny poziom rekursji dla obiektu `std::recursive_mutex` nie jest zdefiniowany, ale po przekroczeniu tej wartości:

* z metody `lock()` rzucony zostanie wyjątek ``std::system_error``
* metoda `try_lock()` zwróci `false`

### TimedLockable i klasa `std::timed_mutex`

Koncept TimedLockable wzbogaca koncept `Lockable` o metody umożliwiające zdefiniowanie maksymalnego czasu oczekiwania na pozyskanie blokady przez wątek

* ``bool try_lock_until(abs_time)``
* ``bool try_lock_for(rel_time)``

Implementacje w bibliotece standardowej - `std::timed_mutex` oraz `std::recursive_timed_mutex`.

## Blokady współdzielone (C++14)

Blokady współdzielone wprowadzone są w standardzie C++14.

Muteksy implementujące koncepty `Lockable` oraz ``TimedLockable`` szeregują odwołania do zasobów, lecz nie rozróżniają dostępów modyfikujących od niemodyfikujących.

Odczyt danych współdzielonych:

* nie może zakłócić innego odczytu współbieżnego
* pozyskiwane są blokady współdzielone

Przed podjęciem próby zapisu danych współdzielonych należy pozyskać blokadę na wyłączność.

### Koncept SharedLockable

Koncept SharedLockable rozszerza koncept `Lockable` o możliwość pozyskiwania blokad współdzielonych (*shared ownership*) przy pomocy metod:

* `void lock_shared()`
* `bool try_lock_shared()`
* `bool try_lock_shared_for(rel_time)`
* `bool try_lock_shared_until(abs_time)`
* `void unlock_shared()`

Implementacja - `std::shared_mutex`

### Klasa `std::shared_mutex` (C++17)

Blokady współdzielone są implementowane przez klasę `std::shared_mutex`

Do pozyskania wyłącznej blokady przed rozpoczęciem operacji zapisu należy użyć:

* `std::lock_guard<std::shared_mutex>`
* `std::unique_lock<std::shared_mutex>`

Do pozyskiwania współdzielonych blokad w trakcie czytania danych należy użyć:

* `std::shared_lock<std::shared_mutex>`

```cpp
int slots[size];
std::shared_mutex mtx_slots;

void reader()
{
    int index = get_slot_index();

    std::shared_lock<std::shared_mutex> lock(mtx_slots); // non-exclusive lock
    int value = slots[index];
    lock.unlock();

    process_value();
}

void writer()
{
    int index = get_slot_index();

    std::lock_guard<std::shared_mutex> lock(mtx_slots); // exclusive lock
    slots[index] = new_value();
}
```

## Menadżery blokad

Zarządzanie blokadami (obiektami muteksów) odbywa się za pomocą następujących klas:

* ``std::lock_guard<Mutex>``
* ``std::unique_lock<Mutex>``
* ``std::shared_lock<Mutex>`` (C++14)

Wszystkie klasy zarządzające blokadami implementują mechanizm *RAII* gwarantujący zwolnienie posiadanej przez wątek blokady w przypadku zgłoszenia sytuacji wyjątkowej.
W destruktorze menadżerów blokad wywoływana jest metoda `unlock()`.

### Klasa ``std::lock_guard<Mutex>``

Menadżer blokad implementuje technikę RAII w celu zarządzania zasobem w postaci muteksu.

```cpp
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

``std::lock_guard`` jest najprostszym managerem blokad:

* konstruktor pozyskuje blokadę wywołując na rzecz muteksu przekazanego przez referencję jako argument metodę `lock()`
* destruktor zwalnia blokadę wywołując `unlock()`
* przeciążony konstruktor przyjmujący jako drugi parametr  obiekt typu ``std::adopt_lock_t`` umożliwia
  adaptowanie, tj. przejęcie prawa własności i tym samym odpowiedzialności za zwolnienie pozyskanej już wcześniej blokady

Przykład:

```cpp
unsigned int counter=0;
std::mutex mtx_counter;

unsigned int increment()
{
    std::lock_guard<std::mutex> lk(mtx_counter);
    return ++counter;
}

unsigned int query()
{
    std::lock_guard<std::mutex> lk(mtx_counter);
    return counter;
}
```

### Klasa ``std::unique_lock<Mutex>``

Rozbudowana wersja managera blokad umożliwiająca:

* ochronę *RAII* przed wyciekami blokad
* opóźnione pozyskiwanie blokad
* adaptowanie pozyskanej przez wątek blokady
* transfer prawa własności - instancja ``unique_lock`` nie jest kopiowalna, ale jest przenaszalna (*moveable*)
* podejmowanie nieblokujących prób pozyskania blokady
* korzystanie z muteksów czasowych

Jeśli instancja managera ``std::unique_lock<>`` posiada blokadę:

* metoda ``mutex()`` zwraca wskaźnik do muteksu realizującego blokadę
* metoda ``owns_lock()`` zwraca wartość ``true``
* w momencie niszczenia obiektu destruktor wywoła metodę `unlock()` na obiekcie muteksu

Opóźnione pozyskiwanie blokady jest możliwe przy pomocy konstruktora ``std::shared_lock(Lockable& m, std::defer_lock_t)``.
Po jego wywołaniu:

* metoda ``owns_lock()`` zwraca `false`
* metoda ``mutex()`` zwraca adres obiektu muteksu
  
Adaptowanie pozyskanej przez wątek blokady - konstruktor ``unique_lock(Lockable& m, std::adopt_lock_t)``

Nieblokujące próby pozyskania blokady - ``unique_lock(Lockable & m,std::try_to_lock_t)``

* w konstruktorze wywoływana jest dla muteksu nieblokująca metoda `try_lock()`
* jeśli blokada zostanie pozyskana metoda ``owns_lock()`` zwraca ``true``, w przeciwnym wypadku zwraca `false`

Jeśli jako parametr szablonu ``unique_lock<Mutex>`` zostanie przekazany typ muteksu implementujący koncept TimedLockable (np. `std::timed_mutex`) możliwe jest blokujące pozyskiwanie blokad:

* albo przez dany czas
* albo do określonego punktu w czasie

```cpp
using namespace std::literals;

std::timed_mutex mutex;
std::unique_lock<std::timed_mutex> lock(mutex, std::try_to_lock);

//...
if (!lock.owns_lock())
{
    int count = 0;
    do
    {
        std::cout << "Thread doesn't own a lock... Tries to acquire a mutex..."
                    << std::endl;
    } while(!lock.try_lock_for(1s));
}
```

#### Transfer własności blokad ``std::unique_lock<Mutex>``

Managery blokad typu ``std::unique_lock<>`` potrafią między sobą transferować własności blokady zgodnie z semantyką
przenoszenia.

```cpp
std::unique_lock<std::mutex> acquire_lock()
{
    static std::mutex m;
    return std::unique_lock<std::mutex>(m);
}
```

## Zakleszczenie

Zakleszczenie (*deadlock*)
    sytuacja, w której co najmniej dwa różne wątki czekają na siebie nawzajem, więc żadny nie może się zakończyć.

Do zakleszczenia może dojść w sytuacji, gdy istnieją przynajmniej dwa zasoby i dwa wątki ubiegające się o dostęp do tych zasobów.

Do zakleszczenia dochodzi, jeśli spełnione są cztery warunki:

1. Wzajemne wykluczenie - w danym czasie tylko jeden wątek może korzystać z zasobu
2. Trzymanie zasobu i oczekiwanie - wątek utrzymuje jeden z zasobów, ale do ukończenia pracy potrzebne jest także zablokowanie innego zasobu
3. Cykliczne oczekiwanie - wątki w taki sposób żądają dostępu do zasobów, że powstaje cykliczny graf skierowany
4. Brak wywłaszczania zasobu - wątki dobrowolnie nie rezygnują z przydzielonych im zasobów, zwolnienie zasobów możliwe jest po zakończeniu zadania

Przykład:

Klasa ze stanem wewnętrznym, chroniona muteksem. Chcemy napisać operator porównania.

```cpp
class X
{
    mutable std::mutex mtx_;
    int some_data_;
public:
    bool operator<(const X& other) const
    {
        std::lock_guard<std::mutex> lk(mtx_);
        std::lock_guard<std::mutex> lk(other.mtx_);
        return some_data_ < other.some_data_;
    }
};
```

Mamy dwa obiekty ``x1`` i ``x2`` i dwa wątki próbujące je porównać, ale w odwrotną stronę:

::::{grid} 2

:::{grid-item-card} Wątek #1
```cpp
if (x1 < x2) { /**/ }
```

jest rozwinięte do: 

```cpp
x1.mtx_.lock(); // 1
/*******************************/
x1.mtx_.lock(); // 3 <- DEADLOCK
```
:::

:::{grid-item-card} Wątek #2
```cpp
if (x2 < x1) { /**/ }
```

jest rozwinięte do:

```cpp
x2.mtx_.lock(); // 2
/*******************************/
x1.mtx_.lock();
```
:::
::::

Dwa wątki pozyskają muteksy w odwrotnej kolejności, co może spowodować zakleszczenie.

### Zapobieganie zakleszczeniom

#### Metoda ``std::lock()``

* gwarantuje zablokowanie wszystkich muteksów bez zakleszczenia niezależnie od kolejności ich pozyskiwania
* wymaga przekazania jako parametrów opóźnionych blokad typu ``std::unique_lock``

```cpp
bool X::operator< (const X& other)
{
    std::unique_lock<std::mutex> l1(mtx_, std::defer_lock);
    std::unique_lock<std::mutex> l2(other.mtx_, std::defer_lock);

    std::lock(l1, l2); // avoiding deadlock
    
    return some_data_ < other.some_data_;
}
```

Istnieje możliwość skonstruowania blokady typu ``std::unique_lock<>`` bez blokowania muteksu za pomocą parametru ``std::defer_lock``.
Pozwala to uniknąć zakleszczeń dla blokad uzyskiwanych jednocześnie. Działa na każdym obiekcie implementującym koncept `Lockable`.

Wciąż może istnieć zagrożenie zakleszczeniem, jeśli blokady są pozyskiwane oddzielnie. Aby zminimalizować ryzyko należy pozyskiwać blokady zawsze w tej samej kolejności.

#### Klasa `std::scoped_lock` - C++17

`std::scoped_lock` to klasa wprowadzona w C++17, która umożliwia wygodne zarządzanie wieloma mutexami w bezpieczny i efektywny sposób. Jest zaprojektowana tak, aby obsługiwać wiele mutexów i zapewnić, że wszystkie zostaną poprawnie zablokowane i odblokowane, unikając zakleszczeń.

```cpp
bool X::operator< (const X& other)
{
    std::scoped_lock l(mtx_, other.mtx_);
    return some_data_ < other.some_data_;
}
```

## Synchronizacja za pomocą zdarzeń

W programach wielowątkowych często zachodzi potrzeba zsynchronizowania pracy wielu wykonywanych współbieżnie zadań.
Dzieje się tak w sytuacji, gdy jeden z pracujących wątków osiąga punkt, w którym nie może wykonać następnych operacji, dopóki inne wątki nie zakończą swojej częsci pracy, przygotowując dane potrzebne do zakończenia zadania. Fakt, że oczekiwane dane są dostępne jest określany jako **zdarzenie** (*event*).

Oczekiwanie na zdarzenie może być zaimplementowane jako:

* *idle wait* - wątek oczekujący na zdarzenie przechodzi do stanu *blocked*, w którym nie zużywa cyklów CPU
* *busy wait* - oczekiwanie na zdarzenie implementowane jest za pomocą pętli *do-while*

### Busy waits - implementacje

#### Implementacja z wykorzystaniem muteksu

Implementacja komunikacji przy pomocy flagi ``data_ready`` typu ``bool`` oraz muteksu:

```cpp
volatile bool data_ready;
std::mutex mtx;
```

::::{grid} 2

:::{grid-item-card} Thread#1

```cpp
void producer()
{
  prepare_data();
    
  std::unique_lock lk(mtx);
  data_ready = true;
}
```

:::

:::{grid-item-card} Thread#2

```cpp
void consumer()
{
  bool ready_flag;

  do
  {
    {
      std::lock_guard lk{mtx};
      ready_flag = data_ready;
    }        
  } while(!ready_flag);

  process_data();
}
```

:::

::::

#### Implementacja z wykorzystaniem zmiennej atomowej

Implementacja wykorzystująca atomową zmienną typu ``std::atomic<bool>`` nie wymaga do synchronizacji za pomocą muteksu.

```cpp
std::atomic<bool> data_ready;
```


::::{grid} 2

:::{grid-item-card} Thread#1

```cpp
void producer()
{
  prepare_data();

  data_ready = true;
}
```

:::

:::{grid-item-card} Thread#2

```cpp
void consumer()
{
  while(!data_ready.load())
  {
    std::this_thread::yield();         
  }

  process_data();
}
```

:::

::::


Operacje na zmiennej atomowej dają gwarancję odpowiedniego uporządkowania kodu zgodnie z modelem pamięci C++11.
Przygotowanie danych - operacja ``prepare_data()``, w której ustawiany jest stan zmiennych współdzielonych, odbywa się przed (*sequenced before*) operacją zapisu do atomowej flagi ``data_ready``.
Kiedy w innym wątku wartość odczytana z zmiennej atomowej ``data_ready`` jest ``true``, operacja zapisu do zmiennej atomowej
synchronizuje się (*synchronizes with*) z tym odczytem tworząc relację *happens before*. Ponieważ relacja *happens before* jest przechodnia, przygotowanie danych odbywa się przed zmianą stanu flagi, ta odbywa się przed odczytem wartości ``true``, który z kolei odbywa się przed przetworzeniem danych.

### Idle waits

#### Zmienne warunkowe

Aby uniknąć aktywnego odpytywania się o spełnienie określonego warunku, efektywniej jest zablokować oczekujący wątek do momentu zajścia zdarzenia (spełnienia oczekiwanego warunku).
Mechanizm taki jest wykorzystany w implementacji *zmiennych warunkowych* (**condition variables**).

Biblioteka standardowa C++11 dostarcza dwie implementacje zmiennych warunkowych:

* `std::condition_variable`
* `std::condition_variable_any`

Obie klasy współpracują z muteksem, aby zapewnić prawidłową synchronizację wątków:

* ``conditional_variable`` współpracuje tylko z typem `std::mutex`
* ``conditional_variable_any`` współpracuje z dowolnym typem muteksu

```cpp
bool data_ready = false;
std::mutex mtx_data_ready;
std::condition_variable cv_is_data_ready;
```

::::{grid} 2

:::{grid-item-card} Thread#1

```cpp
void producer()
{
  prepare_data();

  {
    std::lock_guard lk(mtx_data_ready);
    data_ready = true;
  } // release lock on mutex

  cv_is_data_ready.notify_one();
}
```

:::

:::{grid-item-card} Thread#2

```cpp
void consumer()
{
  std::unique_lock lk(mtx_data_ready);
  while(!data_ready)
  {
    cv_is_data_ready.wait(lk);
  }

  process_data();
}
```

:::

::::


Można uprościć kod oczekujący na spełnienie warunku korzystając z przeciążonej wersji metody `wait(lock_type& lock, predicate_type pred)` przyjmującej jako argument predykat.

Predykat może być zdefiniowany jako:

* funkcja ``bool pred()``
* obiekt funkcyjny
* obiekt funkcyjny zdefiniowany w momencie wywołania przy pomocy funkcji lambda

```cpp
void process_data()
{
    std::unique_lock<std::mutex> lk(mtx_data_ready);
    cv_is_data_ready_.wait(lk, [] { return data_ready; });
    process_data();
}
```

Powiadomienie o zdarzeniu może zostać zrealizowane przy pomocy dwóch metod:

* `void notify_one()` – odblokowuje jeden z wątków znajdujących się w stanie oczekiwania po uprzednim wywołaniu na obiekcie zmiennej warunkowej metody `wait()`
* `void notify_all()` – odblokowuje wszystkie wątki znajdujące się w stanie oczekiwania

#### Zmienne atomowe - C++20

W C++20 wprowadzono możliwość oczekiwania na zmianę wartości zmiennej atomowej za pomocą metody `wait()`. Umożliwia to, w prostych przypadkach, zastąpienie zmiennych warunkowych zmiennymi atomowymi.

```cpp
std::atomic<bool> data_ready = false;
void prepare_data();
void process_data();
```

::::{grid} 2

:::{grid-item-card} Thread#1

```cpp
void producer()
{
  prepare_data();
  
  data_ready = true;
  data_ready.notify_one();
}
```

:::

:::{grid-item-card} Thread#2

```cpp
void consumer()
{
  data_ready.wait(false); // idle wait

  process_data();
}
```

:::

::::


## Lokalna pamięć wątku

* **Thread Local Storage** - dane, które należą do konkretnego wątku
* Każdy wątek może posiadać własny zestaw zmiennych TLS
* Odpowiednik statycznych składowych klasy dla wątków
* Zastosowanie:

  * errno
  * lokalny generator liczb losowych
  * strtok

### Modyfikator ``thread_local``

* C++11 zapewnia dostęp do danych TLS za pomocą modyfikatora ``thread_local``

```cpp
namespace Unsafe
{
    double rand()
    {
        static int seed = 665; // shared state 

        seed = (seed * IMUL + IADD) & MASK;
        return (seed * SCALE);
    }
}

namespace ThreadSafe
{
    double rand_double()
    {
        auto thread_id = std::this_thread::get_id();
        std::hash<std::thread::id> hasher;
        thread_local int thd_seed = 665 * hasher(thread_id);

        int result = (thd_seed * IMUL + IADD) & MASK;
        thd_seed = result;
        return (result * SCALE);
    }
}

int main()
{
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

## Mechanizmy synchronizacji w C++20

### `std::latch`

Klasa `std::latch` pozwala zdefiniować licznik, który może być użyty do synchronizacji wątków. Wartość licznika jest ustawiana podczas konstrukcji obiektu. Wątki mogą czekać na osiągnięcie wartości `0` przez licznik.

Podstawowe metody klasy `std::latch`, to:

* `wait()` - blokuje wątek do momentu osiągnięcia wartości `0` przez licznik
* `arrive_and_wait()` - dekrementuje licznik i blokuje wątek do momentu osiągnięcia wartości `0` przez licznik
* `count_down()` - dekrementuje licznik bez blokowania wątku
* `try_wait()` - sprawdza wartość licznika i zwraca `true`, jeśli jest równa `0` (może sporadycznie zwrócić `false`)

Mechanizm synchronizacji oferowany przez `std::latch` jest przydatny w testach, gdy chcemy mieć pewność, że wszystkie wątki wystartowały:

```cpp
void background_work(const int id, const Data data, 
                     std::latch& all_ready, std::latch& all_done) 
{
    all_ready.arrive_and_wait(); // wait for all threads to be ready
    
    // do some work
    process_data(data);

    all_done.count_down(); // signal that thread has finished
}
```

```cpp
void test_latch()
{
    std::latch all_ready(4);
    std::latch all_done(4);
    std::vector<std::jthread> thds;
    std::vector<Data> data = { /* data */ };

    for(size_t i = 1; i <= 4; ++i)
        thds.emplace_back([id = i, &all_ready, &all_done]{ 
            background_work(id, data, all_ready, all_done); });

    all_done.wait();
}
```

### `std::barrier`

Klasa `std::barrier` pozwala na zablokowanie grupy wątków o znanym rozmiarze do momentu, aż wszystkie wątki z tej grupy osiągną punkt synchronizacji (barierę). W odróżnieniu od `std::latch`, `std::barrier` pozwala na wielokrotne użycie. 

Obiekt bariery przechodzi w trakcie swojego życia kilka faz. Każda faza definiuje tzw. *phase synchronization point* - punkt, w którym czekające wątki są blokowane. Wątki mogą dojść do bariery i opóźnić czekanie na *phase synchronization point* wywołując metodę `arrive()`. Taki wątek może później wywołać metodę `wait()` w celu zablokowania się na *phase synchronization point*.



Obiekt bariery przechodzie przez następujące fazy:

1. Licznik bariery jest dekrementowany przez każde wywołanie `arrive()` lub `arrive_and_drop()`

2. Kiedy wartość licznika osiąga zero, wywoływana jest tzw. *phase completion step*

    * uruchamiana jest funkcja `CompletionFunction`
    * wraz z zakończeniem funkcji `CompletionFunction` odblokowywane są wszystkie czekające wątki

3. Wraz z końcem *phase completion step* licznik jest resetowany do wartości ustawionej w konstruktorze bariery, pomniejszonej o ilość wywołań `arrive_and_drop()`

```cpp
#include <barrier>
#include <iostream>
#include <string>
#include <syncstream>
#include <thread>
#include <vector>
 
int main()
{
    const auto workers = {"John", "Eva", "Carl"};
 
    auto on_completion = []() noexcept
    {
        // locking not needed here
        static auto phase =
            "... done\n"
            "Cleaning up...\n";
        std::cout << phase;
        phase = "... done\n";
    };
 
    std::barrier sync_point(std::ssize(workers), on_completion);
 
    auto work = [&](std::string name)
    {
        std::string product = "  " + name + " worked\n";
        std::osyncstream(std::cout) << product;  // ok, op<< call is atomic
        sync_point.arrive_and_wait(); // wait for all threads to arrive
 
        product = "  " + name + " cleaned\n";
        std::osyncstream(std::cout) << product;
        sync_point.arrive_and_wait(); // wait for all threads to arrive
    };
 
    std::cout << "Starting...\n";
    std::vector<std::jthread> threads;
    threads.reserve(std::size(workers));

    for (auto const& worker : workers)
        threads.emplace_back(work, worker);
}
```

Potencjalny efekt działania programu:

```{code}
Starting...
  John worked
  Carl worked
  Eva worked
... done
Cleaning up...
  John cleaned
  Eva cleaned
  Carl cleaned
... done
```

### Semafory

Semafory są mechanizmem synchronizacji wątków, który pozwala na kontrolowanie dostępu do zasobów. Semafory mogą być używane do synchronizacji dostępu do zasobów współdzielonych, takich jak bufor, pamięć współdzielona, itp.

#### `std::counting_semaphore` - semafor zliczający

Klasa `std::counting_semaphore` reprezentuje semafor zliczający, który pozwala na kontrolowanie dostępu do zasobów. Semafor zliczający pozwala na dostęp do zasobów w ilościach większych niż jeden (wartość licznika jest podawana jako parameter szablonu `LeastMaxValue`).

Obiekt semafora zliczającego zawiera wewnętrzny licznik inicjowany w konstruktorze. Wartość licznika jest zwiększana przez wywołanie metody `release()` i zmniejszana przez wywołanie metody `acquire()`. Kiedy wartość licznika wynosi zero, wątki wywołujące `acquire()` są blokowane do momentu, aż wartość licznika zostanie zwiększona przez wywołanie `release()`.

```cpp
#include <iostream>
#include <thread>
#include <semaphore>

std::counting_semaphore<3> semaphore(3); // Initialize with 3 resources

void worker(int id) 
{
    semaphore.acquire();
    std::cout << "Thread " << id << " acquired the semaphore" << std::endl;

    // do some work with the resource
    process_data();
    

    std::cout << "Thread " << id << " released the semaphore" << std::endl;
    semaphore.release();
}

int main() 
{
    std::thread threads[5];

    // Create and start threads
    for (int i = 0; i < 5; ++i) {
        threads[i] = std::thread(worker, i);
    }

    // Join threads
    for (int i = 0; i < 5; ++i) {
        threads[i].join();
    }

    return 0;
}
```

#### `std::binary_semaphore` - semafor binarny

Klasa `std::binary_semaphore` reprezentuje semafor binarny, który pozwala na kontrolowanie dostępu do zasobów. Semafor binarny pozwala na dostęp do zasobów w ilościach 0 lub 1.

```cpp
#include <chrono>
#include <iostream>
#include <semaphore>
#include <thread>
 
// global binary semaphore instances
// object counts are set to zero
// objects are in non-signaled state
std::binary_semaphore
    smph_signal_main_to_thread{0},
    smph_signal_thread_to_main{0};
 
void background_work()
{
    // wait for a signal from the main proc
    // by attempting to decrement the semaphore
    smph_signal_main_to_thread.acquire();
 
    // this call blocks until the semaphore's count
    // is increased from the main proc
 
    std::cout << "[thread] Got the signal\n"; // response message
 
    // wait for 3 seconds to imitate some work
    // being done by the thread
    using namespace std::literals;
    std::this_thread::sleep_for(3s);
 
    std::cout << "[thread] Send the signal\n"; // message
 
    // signal the main proc back
    smph_signal_thread_to_main.release();
}
 
int main()
{
    // create some worker thread
    std::jthread worker(background_work);
 
    std::cout << "[main] Send the signal\n"; // message
 
    // signal the worker thread to start working
    // by increasing the semaphore's count
    smph_signal_main_to_thread.release();
 
    // wait until the worker thread is done doing the work
    // by attempting to decrement the semaphore's count
    smph_signal_thread_to_main.acquire();
 
    std::cout << "[main] Got the signal\n"; // response message
}
```