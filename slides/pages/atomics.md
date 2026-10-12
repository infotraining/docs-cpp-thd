---
layout: cover
background: /img/header-bg.svg
---

# Typy i zmienne atomowe

---

## Sequential consistency

Defined by Leslie Lamport as 

> "… the result of any execution is the same as if the reads and writes occurred in some order, and the operations of each individual processor appear in this sequence in the order specified by its program"

---

## Spójność sekwencyjna

* Spójność sekwencyjna w programach wielowątkowych, które są wykonywane na maszynach wieloprocesorowych wymaga generowania dodatkowych ograniczeń dotyczących operacji na pamięci (np. barier pamięci - *memory fence*)
* Ponieważ ograniczenia te znacznie obniżają wydajność często wymagane jest poluzowanie tych ograniczeń przy zachowaniu prawidłowego (thread-safe) przebiegu programu

---

## Wyścig - Race condition (data race)

* W sytuacji, kiedy ewaluacja jednego wyrażenia zapisuje wartość zmiennej (*memory location*) i jednocześnie inne ewaluowane wyrażenie, modyfikuje lub odczytuje tą samą zmienną, powstaje konflikt
* Program, który posiada takie dwie konfliktowe ewaluacje wyrażeń jest w sytuacji wyścigu (*data race*), chyba, że:
  * oba wyrażenia są operacjami atomowymi (`std::atomic<T>`)
  * jedno z wyrażeń poprzedza (*happens-before*) drugie (`std::memory_order`)

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

---

## Busy wait - przykład transformacji

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

## Algorytm Dekkera

<img src="/img/atomics/dekker-1.png" class="h-90 mx-auto bg-white p-4"/>

---

## Algorytm Dekkera

<img src="/img/atomics/dekker-2.png" class="h-90 mx-auto bg-white p-4"/>

---

## Algorytm Dekkera

<img src="/img/atomics/dekker-3.png" class="h-90 mx-auto bg-white p-4"/>

---

## Algorytm Dekkera

<img src="/img/atomics/dekker-4.png" class="h-90 mx-auto bg-white p-4"/>

---

## Algorytm Dekkera

<img src="/img/atomics/dekker-5.png" class="h-90 mx-auto bg-white p-4"/>

---

## Algorytm Dekkera

<img src="/img/atomics/dekker-6.png" class="h-90 mx-auto bg-white p-4"/>

---

## Algorytm Dekkera

<img src="/img/atomics/dekker-7.png" class="h-90 mx-auto bg-white p-4"/>

---

## Algorytm Dekkera

<img src="/img/atomics/dekker-8.png" class="h-90 mx-auto bg-white p-4"/>


---

## Ochrona przed wyścigiem (data race)

* C++11 umożliwia uniknięcie wyścigu poprzez stosowanie:
  * Blokad (muteks + menadżer blokady)
  * Zmiennych typów atomowych

---

## Zmienne atomowe

* Operacje wykonywane na zmiennych atomowych:
  * są niepodzielne (*all or nothing*) - żaden inny wątek nie może zobaczyć pośredniego stanu operacji atomowej
  * wprowadzają mechanizm synchronizujący - nie powodują wyścigu
  * ostrzegają kompilator przed potencjalnym wyścigiem - w rezultacie kompilator rezygnuje z niebezpiecznych w takim kontekście optymalizacji (ochrona przed reorderingiem)

---

## Busy wait - poprawna wersja

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

---

## Busy wait - z metodami store/load

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

---

## Typy atomowe

* Standardowe typy atomowe są zdefiniowane w nagłówku `<atomic>`
* Wszystkie operacje zdefiniowane dla tych typów są atomowe
* Do implementacji atomowych operacji mogą być wykorzystane muteksy
  * można to sprawdzić:
    - metodą `x.is_lock_free()`
    - metodą statyczną `X::is_always_lock_free()`

---

## std::atomic_flag

* Atomowa flaga typu *Boolean*
* Standard wymusza, aby wszystkie operacje wykonywane na niej były *lock-free*
* Obiekty typu `std::atomic_flag` są inicjalizowane to stanu *clear*
* Operacja `test_and_set()` umożliwia ustawienie flagi wraz z odpytaniem się o status
* Operacja `clear()` czyści flagę

---

## Szablon std::atomic\<T\>

* Reszta typów atomowych jest dostępna za pomocą specjalizacji szablonu `std::atomic<>`
* Ze specjalizacją dla danego typu związany jest określony alias
  - `atomic_bool` `->` `std::atomic<bool>`
  - `atomic_int` `->` `std::atomic<int>`
* Dla danego typu `T` specjalizacja udostępnia określone operatory

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

## std::atomic_flag

* Jest najprostszym typem atomowym, który reprezentuje *flagę boolowską*
* Obiekty mogą być tylko w jednym z dwóch stanów:
  - `set`
  - lub `clear`

---

## std::atomic_flag - użycie

* Obiekty `std::atomic_flag` muszą być zainicjowane `ATOMIC_FLAG_INIT`
* `clear()` czyści stan obiektu
* `test_and_set()` - operacja RMW

```cpp
std::atomic_flag f = ATOMIC_FLAG_INIT;

f.clear(std::memory_order_release);
bool prev_state = f.test_and_set();
```

---

## Spin Lock

```cpp
class SpinLockMutex
{
    std::atomic_flag flag_;
public:
    SpinLockMutex(): flag_(ATOMIC_FLAG_INIT)
    {}

    void lock()
    {
        while(flag_.test_and_set(std::memory_order_acquire));
    }

    void unlock()
    {
        flag_.clear(std::memory_order_release);
    }
};
```

---

## Operacje na std::atomic\<bool\> - 1

```cpp
std::atomic<bool> b{true};

b = false; // == b.store(false, std::memory_order_seq_cst);
bool current_state = b; // == b.load(std::memory_order_seq_cst);
```

```cpp
extern std::atomic<bool> b;

bool x = b.load(std::memory_order_acquire);

b.store(true); // == b.store(true, std::memory_order_seq_cst);

x = b.exchange(false, std::memory_order_acq_rel);
```

---

## Operacje compare_exchange na std::atomic\<bool\>

* Operacja `compare_exchange` jest fundamentalną konstrukcją dla typów atomowych
* Porównuje bieżącą wartość zmiennej atomowej z wartością dostarczoną jako argument `expected` i gdy wartości są równe zapisuje wartość `desired` tylko w przypadku, zwracając `true`
* Jeśli wartości porównywane nie są równe, wartość `expected` jest uaktualniona i zwracana jest wartość `false`

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

```cpp
bool expected = false;

extern atomic<bool> b; // set somewhere else

if (b.compare_exchange_strong(expected, true))
    std::cout << "b was set by me\n";
else
    std::cout << "b was set by someone else\n";
```

---

## compare_exchange_weak

```cpp
bool expected = false;
extern atomic<bool> b; // set somewhere else

while (!b.compare_exchange_weak(expected, true) && !expected);
```

* Dla operacji `compare_exchange_weak()` zapis wartości może się nie udać nawet wtedy, gdy bieżąca wartość zmiennej jest równa wartości oczekiwanej (tzw. *spurious failure*). Zwracana jest wtedy wartość `false`
* W efekcie taka operacja musi zostać wykonana w pętli
* Taka sytuacja występuje zwykle na maszynach, które nie mają pojedynczej instrukcji *compare-and-exchange*
