---
layout: cover
background: /img/header-bg.svg
---

# Typy i zmienne atomowe

---

## Spójność sekwencyjna - Sequential consistency

* To najsilniejszy i najprostszy **model pamięci**, w którym wszystkie operacje atomowe w programie **wydają się wykonywać w jednej, globalnej kolejności**, wspólnej dla wszystkich wątków

* Intuicyjny model myślenia o współbieżności — dokładnie tak, jakby program był wykonywany na jednym procesorze, krok po kroku

* Zdefiniowana została w 1979 roku przez Leslie Lamporta jako 

  > "… the result of any execution is the same as if the reads and writes occurred in some order, and the operations of each individual processor appear in this sequence in the order specified by its program"

---

## Wyścig - Race condition - Data race

* **Data race** występuje, gdy dwa lub więcej wątków wykonuje operacje na tym samym obiekcie (**memory location**), z których co najmniej jedna jest zapisem, i nie ma między nimi relacji synchronizacji (**happens-before**).

---

## Transformacje kodu

* Kod programu, może zostać poddany transformacjom (*reordering*) przez
  * kompilator (optymalizacja)
  * procesor (prefetching)
  * cache system (store buffers)

---
layout: center
---

## Transformacje na wszystkich poziomach są równoważne!!!

---

## Busy wait

<!-- old snippet
```cpp
int data{0};
bool is_data_ready{false};
```

<div class="grid grid-cols-2 gap-4">
<div>

```cpp
//Thread#1

x = 42;

is_data_ready = true;
```

</div>
<div>

```cpp
//Thread#2

while (!is_data_ready)
{}

assert(x == 42);
```

</div>
</div>
-->

<<< @/../examples/12_atomics.cpp#race_decl

<div class="grid grid-cols-2 gap-4">
<div>

<<< @/../examples/12_atomics.cpp#race_thd1 {all}

</div>
<div>

<<< @/../examples/12_atomics.cpp#race_thd2 {all}

</div>
</div>

---

## Busy wait - przykład transformacji

<!-- old snippet
```cpp
int data{0};
bool is_data_ready{false};
```

<div class="grid grid-cols-2 gap-4">
<div>

```cpp
//Thread#1

x = 42;

is_data_ready = true;
```

</div>
<div>

```cpp {all|3-4|6}
//Thread#2

bool tmp = is_data_ready;
while (!tmp) {}

assert(x == 42);
```

transformacja dokonana przez kompilator - assert fails

</div>
</div>
-->

<<< @/../examples/12_atomics.cpp#race_decl

<div class="grid grid-cols-2 gap-4">
<div>

<<< @/../examples/12_atomics.cpp#race_thd1 {all}

</div>
<div>

<<< @/../examples/12_atomics.cpp#race_transformed_thd2 {all|3-6|8}

transformacja dokonana przez kompilator - assert fails

</div>
</div>

---

## Algorytm Dekkera

```cpp
int flag_1 = 0;
int flag_2 = 0;
```

<div class="grid grid-cols-2 gap-4">
<div>

```cpp
//Thread#1

flag_1 = 1;

if (flag_2 != 0)
  // resolve contention
else
  // enter critical section
```

</div>
<div>

```cpp
//Thread#2

flag_2 = 1;

if (flag_1 != 0)
  // resolve contention
else
  // enter critical section
```

</div>
</div>

---
class: white-slide
---

## Algorytm Dekkera

<img src="/img/atomics/dekker-1.png" class="h-90 mx-auto bg-white p-4"/>

---
class: white-slide
---

## Algorytm Dekkera

<img src="/img/atomics/dekker-2.png" class="h-90 mx-auto bg-white p-4"/>

---
class: white-slide
---

## Algorytm Dekkera

<img src="/img/atomics/dekker-3.png" class="h-90 mx-auto bg-white p-4"/>

---
class: white-slide
---

## Algorytm Dekkera

<img src="/img/atomics/dekker-4.png" class="h-90 mx-auto bg-white p-4"/>

---
class: white-slide
---

## Algorytm Dekkera

<img src="/img/atomics/dekker-5.png" class="h-90 mx-auto bg-white p-4"/>

---
class: white-slide
---

## Algorytm Dekkera

<img src="/img/atomics/dekker-6.png" class="h-90 mx-auto bg-white p-4"/>

---
class: white-slide
---

## Algorytm Dekkera

<img src="/img/atomics/dekker-7.png" class="h-90 mx-auto bg-white p-4"/>

---
class: white-slide
---

## Algorytm Dekkera

<img src="/img/atomics/dekker-8.png" class="h-90 mx-auto bg-white p-4"/>


---

## Ochrona przed wyścigiem (data race)

<v-clicks>

* Brak/Obecność relacji **happens-before** jest kluczowym kryterium dla określenia, czy występuje **data race**

* C++ Memory Model umożliwia synchronizaję poprzez relacje **happens-before**, co pozwala uniknąć wyścigów danych

* Synchronizacja może pochodzić z:
  * Blokad mutexów (`lock() / unlock()`)
  * Operacji na zmiennych atomowych (`store() / load()`)
  * Zmiennych warunkowych (`wait() / notify_one() / notify_all()`)
  * Operacji na pamięci współdzielonej z użyciem barier pamięci (`atomic_thread_fence()`)
  * Start wątku (`std::thread` constructor) / Dołączenie wątku (`std::thread::join()`)

</v-clicks>

---

## Zmienne atomowe

* Operacje wykonywane na zmiennych atomowych:
  * są niepodzielne (*all or nothing*) - żaden inny wątek nie może zobaczyć pośredniego stanu operacji atomowej
  * wprowadzają mechanizm synchronizujący - nie powodują wyścigu
  * ostrzegają kompilator przed potencjalnym wyścigiem - w rezultacie kompilator rezygnuje z niebezpiecznych w takim kontekście optymalizacji (ochrona przed reorderingiem)

---

## Busy wait - poprawna wersja

<!-- old snippet
```cpp
int data{0};
std::atomic<bool> is_data_ready{false};
```

<div class="grid grid-cols-2 gap-4">
<div>

```cpp
//Thread#1

x = 42;

is_data_ready = true; 
```

</div>
<div>

```cpp
//Thread#2

while (!is_data_ready)
{}

assert(x == 42);
```

</div>
</div>
-->

<<< @/../examples/12_atomics.cpp#atomic_decl

<div class="grid grid-cols-2 gap-4">
<div>

<<< @/../examples/12_atomics.cpp#atomic_thd1 {all}

</div>
<div>

<<< @/../examples/12_atomics.cpp#atomic_thd2 {all}

</div>
</div>

---

## Busy wait - z metodami store/load

<!-- old snippet
```cpp
int data{0};
std::atomic<bool> is_data_ready{false};
```

<div class="grid grid-cols-2 gap-4">
<div>

```cpp
//Thread#1

x = 42;

is_data_ready.store(true); 
```

</div>
<div>

```cpp
//Thread#2

while (!is_data_ready.load())
{}

assert(x == 42);
```

</div>
</div>
-->

<<< @/../examples/12_atomics.cpp#atomic_decl

<div class="grid grid-cols-2 gap-4">
<div>

<<< @/../examples/12_atomics.cpp#store_load_thd1 {all}

</div>
<div>

<<< @/../examples/12_atomics.cpp#store_load_thd2 {all}

</div>
</div>

---

## Typy atomowe

* Standardowe typy atomowe są zdefiniowane w nagłówku `<atomic>`
* Wszystkie operacje zdefiniowane dla tych typów są niepodzielne (atomowe)
* Do implementacji atomowych operacji mogą być wykorzystane muteksy
  * można to sprawdzić:
    - metodą `x.is_lock_free()`
    - metodą statyczną `X::is_always_lock_free()`

---

## std::atomic_flag

* Atomowa flaga typu *Boolean*
* Standard wymusza, aby wszystkie operacje wykonywane na niej były *lock-free*
* Obiekty typu `std::atomic_flag` są inicjalizowane to stanu *clear*
* Interfejs:
  * `bool test_and_set()` - ustawia flagę i zwraca jej poprzedni stan
   * `void clear()` - czyści flagę

---

## std::atomic_flag - przykład użycia


<<< @/../examples/12_atomics.cpp#atomic_flag_usage

---

## Spin Lock

* Implementacja prostego spin locka przy użyciu `std::atomic_flag`

<div class="text-code-08">

<<< @/../examples/12_atomics.cpp#spin_lock {all|3|10-12|17}

</div>

---

## Szablon std::atomic\<T\>

* Reszta typów atomowych jest dostępna za pomocą specjalizacji szablonu `std::atomic<>`
* Ze specjalizacją dla danego typu związany jest określony alias
  - `atomic_bool` `->` `std::atomic<bool>`
  - `atomic_int` `->` `std::atomic<int>`
* Dla danego typu `T` specjalizacja udostępnia określone operatory
  - np. `operator++` dla `std::atomic<int>`

---

## std::atomic\<T\>

* Obiekty atomowe są niekopiowalne i nieprzenaszalne
* Wspierają przypisanie z i niejawną konwersję do odpowiedniego typu podstawowego

---

## std::atomic\<T\> - interfejs podstawowy

* Podstawowe operacje dla wszystkich typów atomowych:
  * `T load(std::memory_order)`
  * `void store(T desired, std::memory_order)`
  * `T exchange(T desired, std::memory_order)`
  * `bool compare_exchange_weak(T& expected, T desired, std::memory_order)`
  * `bool compare_exchange_strong(T& expected, T desired, std::memory_order)`

---

## std::atomic\<T\> - interfejs rozszerzony

* Wybrane typy wspierają dodatkowo operacje:
  * `T fetch_add(T arg, std::memory_order)`
  * `T fetch_sub(T arg, std::memory_order)`
  * `T fetch_or(T arg, std::memory_order)`
  * `T fetch_xor(T arg, std::memory_order)`

---

## Memory ordering - std::memory_order

* Określa, w jaki sposób operacje atomowe są widoczne dla innych wątków

* Każda operacja atomowa otrzymuje opcjonalny argument typu `std::memory_order`


* Wyliczenie `std::memory_order` ma sześć możliwych wartości:
  * `std::memory_order_relaxed`
  * `std::memory_order_acquire`
  * `std::memory_order_consume`
  * `std::memory_order_acq_rel`
  * `std::memory_order_release`
  * `std::memory_order_seq_cst` - domyślna wartość dla argumentów operacji atomowych

---

## Memory ordering - kategorie operacji

* To jakie wartości wyliczenia są dozwolone dla określonej operacji atomowej zależy od jej kategorii
  * *Store* 
    - `memory_order_relaxed`, `memory_order_release` lub `memory_order_seq_cst`
  * *Load* 
    - `memory_order_consume`, `memory_order_acquire`, `memory_order_seq_cst`
  * *Read-modify-write* 
    - `memory_order_relaxed`, `memory_order_consume`, `memory_order_acquire`, `memory_order_release`, `memory_order_acq_rel` lub `memory_order_seq_cst`

---

## memory_order_relaxed

* Operacja atomowa z `memory_order_relaxed` nie wprowadza żadnych dodatkowych ograniczeń dotyczących kolejności względem innych operacji atomowych

* Zapewnia jedynie niepodzielność samej operacji

* Może być używana, gdy nie zależy nam na synchronizacji z innymi wątkami, a jedynie na bezpiecznym odczycie lub zapisie wartości atomowej - np. liczniki

---

## memory_order_relaxed - przykład

```c++
std::atomic<int> counter{0};

auto incrementer = [&counter]() {
    counter.fetch_add(1, std::memory_order_relaxed);
};

{
    std::jthread t1{incrementer};
    std::jthread t2{incrementer};
} // join provides happens-before relationship

int value = counter.load(std::memory_order_relaxed);
```

---

## memory_order_release / memory_order_acquire

* `memory_order_release` - stosowane przy zapisie wartości atomowej, zapewnia, że wszystkie wcześniejsze operacje w tym wątku są widoczne dla innych wątków przed tym zapisem
* `memory_order_acquire` - stosowane przy odczycie wartości atomowej, zapewnia, że wszystkie późniejsze operacje w tym wątku widzą efekty wszystkich wcześniejszych operacji w innych wątkach, które zakończyły się `memory_order_release` na tej samej zmiennej atomowej

* `memory_order_acq_rel` - stosowane przy operacjach RMW, łączy efekty `memory_order_acquire` i `memory_order_release`

---

## memory_order_release / memory_order_acquire

<div class="text-code-08">

```c++
std::vector<int> data;
std::atomic<bool> data_is_ready{false};

auto writer = [&data]() {
    data = collect_data();
    data_is_ready.store(true, std::memory_order_release); // synchronizes with acquire load in reader 
};                                                        // -> establishes happens-before relationship

auto reader = [&data]() {
    while (!data_is_ready.load(std::memory_order_acquire))
    {} // spin-wait until data is ready - synchronizes with release store in writer

    process_data(data);
};

{
    std::jthread t1{writer};
    std::jthread t2{reader};
}
```

</div>

---

## memory_order_seq_cst - spójność sekwencyjna

* Domyślny i najbardziej restrykcyjny porządek pamięci, zapewniający spójność sekwencyjną wszystkich operacji atomowych w programie

* Każda operacja atomowa z `memory_order_seq_cst` jest częścią globalnego porządku i jest widoczna dla wszystkich wątków w tej samej kolejności

* Ponieważ jest to porządek najmocniejszy:
  * `store(..., std::memory_order_seq_cst)` może zastąpić zapis z `memory_order_release`
  * `load(std::memory_order_seq_cst)` może zastąpić odczyt z `memory_order_acquire`
  * operacje RMW z `memory_order_seq_cst` mogą zastąpić operacje z `memory_order_acq_rel`

---

## Operacje compare_exchange

* Operacja `compare_exchange` jest fundamentalną konstrukcją dla typów atomowych
* Porównuje bieżącą wartość zmiennej atomowej z wartością `expected`; jeśli są równe, zapisuje `desired` i zwraca `true`
* Jeśli wartości są różne, nie zapisuje `desired`, uaktualnia `expected` bieżącą wartością i zwraca `false`
* Porównanie i warunkowy zapis są **jedną niepodzielną operacją read-modify-write**

---

## Po co nam compare_exchange?

* Czasem nowa wartość zależy od aktualnej, a sprawdzenie warunku i zapis muszą być jednym krokiem
* Osobne `load()` i `store()` nie wystarczą: inny wątek może zmienić wartość pomiędzy nimi
* `compare_exchange` zapisuje nową wartość tylko wtedy, gdy zmienna atomowa nadal ma oczekiwaną wartość
* Przy niepowodzeniu `expected` dostaje aktualną wartość — można na jej podstawie policzyć nową i spróbować ponownie

---

## Przykład użycia compare_exchange

```cpp
std::atomic<bool> claimed{false};

// Dwa wątki mogą odczytać false i oba wejść do sekcji:
if (!claimed.load())
{
    claimed.store(true);
    enter_critical_section();
}

// Tylko jeden wątek atomowo zmieni false na true:
bool expected = false;
if (claimed.compare_exchange_strong(expected, true))
    enter_critical_section();
```

---

## compare_exchange_strong - kod konceptualny

```cpp
bool atomic<T>::compare_exchange_strong(T& expected, T desired)
{
    if (this->value == expected ) 
    { 
        this->value = desired; 
        return true; 
    }
    
    expected = this->value;
    return false;
}
```

---

## compare_exchange_strong

<div class="text-m">

* Użyj, gdy pojedyncza próba ma dać jednoznaczny wynik

* `false` oznacza, że wartość atomowa nie była równa `expected`; nie występuje pozorne niepowodzenie (*spurious failure*)

* Operacja może się nie udać, jeśli inny wątek wcześniej zmienił wartość

</div>

<<< @/../examples/12_atomics.cpp#cas_strong

---

## compare_exchange_weak

<div class="text-m">

* Może zwrócić `false` także wtedy, gdy wartości są równe — to pozorne niepowodzenie (*spurious failure*)
* Zwykle używa się jej w pętli, która ponawia próbę i przelicza `desired` dla aktualnego `expected`
* Gdy wartości są różne, `expected` zostaje uaktualnione; pętla może więc sprawdzić nową wartość
* To typowy wzorzec dla aktualizacji, które można bezpiecznie ponawiać — np. ograniczania licznika do limitu

</div>

<div class="text-code-08">

<<< @/../examples/12_atomics.cpp#cas_weak {all}

</div>

---

## Struktury Lock-Free

* Struktury lock-free pozwalają na bezpieczne operacje wielowątkowe bez użycia blokad, wykorzystując atomowe operacje CAS (Compare-And-Swap)
* Dzięki temu można osiągnąć wysoką wydajność w środowiskach wielowątkowych, unikając kosztów związanych z blokadami.
* Mogą być trudniejsze do zaprojektowania i zweryfikowania niż tradycyjne struktury z blokadami.

---

## Rodzaje struktur lock-free

* **SPSC** - Single Producer, Single Consumer
* **MPSC** - Multiple Producers, Single Consumer
* **SPMC** - Single Producer, Multiple Consumers
* **MPMC** - Multiple Producers, Multiple Consumers

---

## Lock-Free Stack - przykład implementacji

* Prealokowana pula węzłów - brak dynamicznej alokacji podczas operacji na stosie
* Brak muteksów - synchronizacja odbywa się wyłącznie za pomocą operacji atomowych CAS
* Brak problemu ABA - problem z podwójnym odczytem i zapisem tego samego węzła (brak zwalniania pamięci)
* MPMC - Multiple Producers, Multiple Consumers

---

## LockFreeStack - rezerwacja węzła

<<< @/../examples/12_atomics.cpp#lockfree_stack_reserve_node {all|3|4-9}{at:1}

<v-switch>
  <template #1>
    <code>allocated_</code> - licznik ile węzłów już przydzielono
  </template>
  <template #2>
    Każdy wątek próbuje zwiększyć licznik o 1
  </template>
  <template #3>
    Jeśli CAS się uda → wątek dostaje węzeł <code>nodes_[index]</code>
    <br/>
    Jeśli CAS się nie uda → index jest aktualizowany i pętla próbuje ponownie.
  </template>
  <template #4>
    <code>memory_order_relaxed</code> jest używane do operacji na liczniku, ponieważ nie wymaga synchronizacji z innymi wątkami - jedyne wymaganie to niepodzielność operacji (atomicity)
  </template>
</v-switch>

---

## LockFreeStack — dodawanie elementu przez CAS

<div class="text-code-09">

<<< @/../examples/12_atomics.cpp#lockfree_stack_push {all|3-6|8|11|9-10,12-13}{at:1}

<v-switch>
  <template #1>
    Najpierw rezerwowany jest nieużywany węzeł z puli
  </template>
  <template #2>
    Odczytujemy aktualny <code>head</code> stosu i zapisujemy go w zmiennej <code>expected</code>
  </template>
  <template #3>
    Ustawiamy <code>node->next = head</code>
  </template>
  <template #4>
    Próbujemy CAS: <code>head → node</code> pod warunkiem, że <code>head</code> nadal wskazuje <code>expected</code>
  </template>
  <template #5>
    Jeśli CAS się nie uda (bo ktoś inny zrobił push/pop) to <code>expected</code> jest aktualizowany i powtarzamy próbę
  </template>
  <template #6>
    <code>memory_order_release</code> przy sukcesie publikuje zapisy dla wątku, który zrobi <code>pop()</code> z <code>memory_order_acquire</code>
  </template>
  <template #7>
    <code>memory_order_relaxed</code> jest używane przy porażce, ponieważ nie wymaga synchronizacji z innymi wątkami - jedyne wymaganie to niepodzielność operacji (atomicity)
  </template>
</v-switch>

</div>

---

## LockFreeStack — zdejmowanie elementu przez CAS

<div class="text-code-09">

<<< @/../examples/12_atomics.cpp#lockfree_stack_pop {all|3|4,14|6|7-8|4-13|7-12|7-8|7-8}{at:1}

<v-switch>
  <template #1>
    Odczytujemy <code>head</code> z <code>memory_order_acquire</code> - gwarantuje widoczność zapisów dokonanych przez inne wątki przed ich zwolnieniem (release)
  </template>
  <template #2>
    Jeśli <code>head == nullptr</code> → stos pusty
  </template>
  <template #3>
    Pobieramy <code>next = head->next</code>
  </template>
  <template #4>
    Próbujemy CAS: <code>head → next</code> pod warunkiem, że <code>head</code> nadal wskazuje <code>expected</code>
  </template>
  <template #5>
    Jeśli CAS się nie uda (bo ktoś inny zrobił push/pop) to <code>expected</code> jest aktualizowany i powtarzamy próbę
  </template>
  <template #6>
  Jeśli CAS się powiedzie, zwracamy zdjęty węzeł
  </template>
  <template #7>
    <code>memory_order_acq_rel</code> przy sukcesie CAS
    <br/>
    <code>acquire</code>: widzimy poprawnie zawartość węzła
    <br/>
    <code>release</code>: zapewnia poprawną publikację zmian w stosie (np. dla innych pop/push).
  </template>
  <template #8>
    <code>memory_order_acquire</code> na porażce CAS
  </template>
</v-switch> 

</div>

---

## Atomics ≠ volatile

* `volatile`:
  * nie zapewnia synchronizacji
  * nie zapewnia niepodzielności operacji (atomicity)
  * służy do Memory-Mapped I/O (MMIO)

* `std::atomic<T>`
  * semantyka typów atomowych zapewnia pełną synchronizację

---

## Reordering

* Kompilator i procesor mogą zmieniać kolejność instrukcji w celu optymalizacji

* Dla zmiennych nieatomowych może to prowadzić do nieoczekiwanych wyników w programach wielowątkowych

* Typy atomowe zapewniają mechanizmy synchronizacji, które ograniczają możliwość zmiany kolejności instrukcji w sposób, który mógłby naruszyć spójność danych

---

## Reordering - Hardware

* Store buffers - mogą powodować, że zapis do pamięci nie jest natychmiast widoczny dla innych wątków, nawet jeśli operacja została wykonana w kolejności programowej

* Invalidation queues - mogą powodować, że zmiany w pamięci podręcznej nie są natychmiast widoczne dla innych wątków

* Pipelining - może powodować, że instrukcje są wykonywane w kolejności różnej od kolejności programowej, co może wpływać na widoczność zmian w pamięci dla innych wątków

---

## Reordering - Kompilator

* Kompilator może zmieniać kolejność instrukcji w celu optymalizacji kodu

* Rodzaje optymalizacji kompilatora obejmują:
  * Przesuwanie instrukcji w górę lub w dół w celu zmniejszenia liczby opóźnień
  * Łączenie instrukcji w celu zmniejszenia liczby operacji pamięciowych - 
  * Usuwanie nieużywanych zmiennych i instrukcji
  * Caching - kompilator może zmieniać sposób korzystania z pamięci podręcznej w celu optymalizacji dostępu do danych
  
