***************
Zmienne atomowe
***************

Problemy architektur wieloprocesorowych
=======================================

* **Cache coherency**

  Wątki T1 i T2, wykonywane na różnych procesorach/rdzeniach odczytują dane A i B, które znajdują się w tej samej linii cache'a L2. Jeśli następnie
  wątek T1 modyfikuje wartość A, to modyfikacja jest najpierw dokonana w cache'u L2 procesora na którym pracuje wątek. Powstaje problem, ponieważ wątek
  T2 pracuje na innym procesorze, który w swoim cache'u ma nieaktualną już wartość danej A. Cache stają się niekoherentne i należy je uaktualnić.
  Za spójność cache odpowiedzialny jest protokół MESI.

  - *False sharing* - mechanizm gwarantujący spójność danych w cache'ach może czasami znacznie spowolnić działanie aplikacji wielowątkowej. 
    Jeśli pracujące wątki nie współdzielą danych, lecz dane te fizycznie leżą blisko siebie, to mogą one znaleźć się na tej samej lini cache'a w procesorach. W rezultacie jakakolwiek modyfikacja stanu zmiennych (teoretycznie niezależnych od siebie) wymusza inwalidację cache'ów.
    

* **Memory consistency**
  
  Protokół spójności cache'a nie daje gwarancji, kiedy modyfikacja wartości (operacja zapisu nowego stanu) zostanie zakończona. Powstaje pytanie: kiedy
  uaktualniona przez jeden wątek wartość będzie widoczna w pozostałych wątkach pracujących na innych procesorach.
  W zachowaniu spójności pamięci pomocny jest model pamięci wprowadzony w C++11.

Kluczowe pojęcia
================

* Spójność sekwencyjna - *Sequential consistency*
  
  .. epigraph::
  
     "The result of any execution is the same as if the reads and writes occurred in some order, and the operations of each individual processor appear in this sequence in the order specified by its program"
  
     -- Leslie Lamport, 1979
  
  Spójność sekwencyjna w programach wielowątkowych, które są wykonywane na maszynach wieloprocesorowych wymaga generowania dodatkowych ograniczeń 
  dotyczących operacji na pamięci (np. barier pamięci - **memory fence**). Ponieważ ograniczenia te znacznie obniżają wydajność często wymagane jest 
  poluzowanie tych ograniczeń przy zachowaniu prawidłowego (*thread-safe*) przebiegu programu.


* Wyścig - *Race condition (data race)*
  

  W sytuacji, kiedy ewaluacja jednego wyrażenia zapisuje wartość zmiennej (**memory location**) i jednocześnie inne ewaluowane wyrażenie modyfikuje
  lub odczytuje tą samą zmienną, powstaje **konflikt**. Program, który posiada dwie konfliktowe ewaluacje wyrażeń jest w sytuacji wyścigu (**data race**), chyba, że:

  - oba wyrażenia są operacjami atomowymi (``std::atomics``)
  - jedno z wyrażeń poprzedza (**happens-before**) drugie (``std::memory_order``)

  .. warning:: Jeśli w programie pojawia się wyścig (*data race*), zachowanie programu staje się niezdefiniowane (*undefined behaviour*)
        

* Relacja **happens-before**
  
  - Dostęp do pamięci (*memory access*) A poprzedza (*happens-before*) B, jeżeli:
    
    - A poprzedza B w programie
    - A i B są operacjami synchronizującymi i B obserwuje rezultat A, narzucając kolejność operacji
      
      + Przykład: A zwalnia muteks m, B następnie pozyskuje muteks m
          
        .. image:: _images/happens-before-mutex.*
        
    
    - lub istnieje C takie, że A poprzedza (*happens-before*) C i C poprzedza B
      
  - Dwie operacje dostępu do pamięci (*memory location*) A i B biorą udział w wyścigu (*data race*), jeżeli:

    + A nie poprzedza B
    + ani B nie poprzedza A

Transformacje programu
======================

Analizę kodu wielowątkowego komplikuje fakt, że kod wykonywany może zostać poddany transformacji.

.. note:: Transformacje to zmiana kolejności zapisów i odczytów.

Transformacje mogą zachodzić na dowolnym poziomie:

#. **kod źródłowy**
   
   - optymalizacja dokonana przez kompilator 
   - reorganizacja operacji odczytów i zapisów

#. **wykonanie** 
   
   - procesor + cache 
   - buforowanie operacji zapisu

Przykłady transformacji powodującej *undefined behaviour*
---------------------------------------------------------

Busy wait
^^^^^^^^^

.. code-block:: c++

    bool done;
    int x;

+---------------------+--------------------------------------------+
| thread 1            | thread 2                                   |
+=====================+============================================+
| | x = 42;           | | while (!done) {}                         |
| | done = true;      | | assert(x == 42);                         | 
+---------------------+--------------------------------------------+

Kompilator może przetransformować ten kod do postaci:

+---------------+-------------------+
| thread 1      | thread 2          |
+===============+===================+
|| x = 42;      || tmp = done;      |
|| done = true; || while (!tmp) {}  |
||              || assert(x == 42); |
+---------------+-------------------+

Ewentualnie kompilator lub hardware (ARM, PowerPC) mogą dokonać następującej transformacji:

+----------------+--------------------+
| thread 1       | thread 2           |
+================+====================+
| | done = true; | | while (!tmp) {}  |
| | x = 42;      | | assert(x == 42); |
+----------------+--------------------+


Dekker's algorithm
^^^^^^^^^^^^^^^^^^

.. code-block:: c++

    int flag1 = flag2 = 0

+----------------------------+------------------------------+
| thread 1                   | thread 2                     |
+============================+==============================+
| | flag_1 = 1        // (a) | | flag_2 = 1        // (c)   |
| | if (flag_2 != 0)  // (b) | | if (flag_1 != 0)  // (d)   |
| |   // contention          | |   // contention            |
| | else                     | | else                       |
| |   // critical section    | |   // critical section      |
+----------------------------+------------------------------+

.. Kiedy przestaje działać?

.. .. image:: pics/atomic/store_buffer.*


Ochrona przed wyścigiem (**data-race**)
=======================================

C++11 umożliwia uniknięcie wyścigu poprzez stosowanie:

#. Blokad (muteks + menadżer blokady)
#. Obiektów **atomowych**

   * operacje wykonywane na zmiennych atomowych:
     
     - są niepodzielne - żaden inny wątek nie może zobaczyć pośredniego stanu operacji atomowej
     - wprowadzają mechanizm synchronizujący - nie powodują wyścigu
     - ostrzegają kompilator przed potencjalnym wyścigiem - w rezutacie kompilator rezygnuje z niebezpiecznych w takim kontekście optymalizacji
  
Stosując atomową flagę typu ``atomic<bool>`` możemy rozwiązać problem implementacji algorytmu *busy-wait*:

.. code-block:: c++

    std::atomic<bool> done;
    int x;

+---------------------+--------------------------------------------+
| thread 1            | thread 2                                   |
+=====================+============================================+
| | x = 42;           | | while (!done) {}                         |
| | done = true       | | assert(x == 42);                         |
+---------------------+--------------------------------------------+

Zastosowanie zmiennej atomowej ``done`` zapewnia odpowiednie porządkowanie operacji dostępu do pamięci.
Zapis do atomowej flagi ``done`` poprzedza (**happens-before**) zapis wartości ``42`` do zmiennej ``x``. 
Kiedy wartość flagi odczytana w drugim wątku ma wartość ``true``, zapis flagi synchronizuje się (**synchronizes-with**) z jej odczytem, tworząc relację
**happens-before**. Ponieważ relacja **happens-before** jest przechodnia wymuszone zostaje następujące uporządkowanie:

* zapis do zmiennej ``x`` poprzedza zapis do flagi ``done``
* zapis do flagi ``done`` poprzedza odczyt wartości ``true``
* operacja odczytu wartości ``true`` poprzedza odczyt zmiennej ``x``

Relacja synchronizacji **synchronizes-with**
--------------------------------------------

.. epigraph::

   An atomic operation A that performs a release operation on an atomic object M synchronizes with an atomic operation B that performs an acquire operation on M and takes its value from any side effect in the release sequence headed by A.

   -- C++11 standard


An atomic operation A that performs a release operation on an atomic object M synchronizes with an atomic operation B that performs an acquire operation on M and takes its value from any side effect in the release sequence headed by A.


Semantyka Acquire-Release
-------------------------

Transakcja = Działanie logiczne na powiązanych danych, które utrzymują niezmienność.

* Atomowo: All-or-nothing.
* Spójnie: Odczyt spójnego stanu lub zmiana stanu w inny spójny stan.
* Niezależnie: Poprawne działanie gdy wykonywane są inne transakcje.

.. code-block:: cpp

    bank_account acct1;
    bank_account acct2;

    // begin transaction - ACQUIRE exclusivity
    acct1.credit( 100 );
    acct2.debit ( 100 );
    // end transaction - RELEASE exclusivity

.. .. image:: pics/atomic/aquire_release.*

Rozwiązanie:

* mutexy
* typy atomowe
* bariery pamięci - (**memory-fence**)


Blokady - **locks**
^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp

    { lock_guard<mutex> hold(mtx);  // enter critical region
                                    //  (lock "acquire")

    ... read/write x ...
    }                               // exit critical region
                                    //  (lock "release")

Ordered atomics
^^^^^^^^^^^^^^^

.. code-block:: cpp

    while( whose_turn != me ) { }   // enter critical region
                                    //  (atomic read "acquires" value)
    ... read/write x ...
    whose_turn = someone_else;      // exit critical region
                                    //  (atomic write "release")

Pamięć transakcyjna (work in progress)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Pamięć transakcyjna (*transactional memory*) umożliwi zgrupowanie szeregu instrukcji
w transakcję, która jest **atomowa** oraz **izolowana**. 

Implementacja może korzystać z wsparcia sprzętowego (na wspieranych architekturach).

Przykład:

.. code-block:: cpp

    // each call to worker() retrieves a unique value of i, even when done in parallel
    int worker()
    {
        static int i = 0;
        atomic_noexcept { // begin transaction
           //  printf("before %d\n", i); // error: cannot call a non transaction-safe function
           ++i;
           return i; // commit transaction
        }
    }                           


Typy atomowe w C++11
====================

Obiekty typów atomowych umożliwiają wykonanie na nich podstawowych operacji (przypisania i odczytu wartości) w niepodzielny sposób oraz zapewniają odpowiednie uporządkowanie operacji dostępu do pamięci. Dzięki tym cechom można uniknąć w kodzie sytuacji wyścigu (*data races*).
Jeśli jeden wątek zapisuje obiekt atomowy, w trakcie gdy drugi go odczytuje, to zachowanie się programu jest zdefiniowane.

``std::atomic_flag``
--------------------

``std::atomic_flag`` jest flagą ``bool``. Jego atomowe zachowanie jest *gwarantowane* przez standard.
Interfejs jest bardzo uproszczony.

``operator=()``
    operator przypisania

``clear()``
    ustawia (atomowo) flagę na ``false``

``test_and_set()``
    ustawia (atomowo) flagę na ``true`` i zwraca poprzednią wartość

Można za pomocą tej flagi skonstruować prosty obiekt blokady, zachowujący się jak muteks.

.. code-block:: cpp

    class SpinLock
    {
        std::atomic_flag flag;
    public:
        SpinLock() : flag(ATOMIC_FLAG_INIT) {}

        bool try_lock()
        {
            return !flag.test_and_set(std::memory_order_acquire);
        }

        void lock()
        {
            while(flag.test_and_set(std::memory_order_acquire));
        }

        void unlock()
        {
            flag.clear(std::memory_order_release);
        }
    };


Klasa ``std::atomic<T>``
------------------------

Klasa szablonowa, generująca typy, które zachowują się "atomowo".

**Interfejs**

* ``bool is_lock_free()``
  
  zwraca ``true``, jeśli operacje wykonywane na obiekcie są *lock-free*

* | ``void store(T desired,`` 
  |              ``std::memory_order order = std::memory_order_seq_cst)``
  
  atomowo zmienia wartość obiektu

* ``T load(std::memory_order order = std::memory_order_seq_cst) const``
  
  atomowo pobiera wartość obiektu

* | ``T exchange(T desired,`` 
  |              ``std::memory_order order = std::memory_order_seq_cst)``
  
  atomowo zmienia wartość obiektu i zwraca poprzednią wartość

* | ``compare_exchange_weak/strong(T& expected, T desired,`` 
  |                                ``std::memory_order success,``
  |                                ``std::memory_order failure)``
  
  atomowo porównuje wartość obiektu z argumentem i wykonuje ``exchange`` jeśli wartość jest równa, lub ``load`` jeśli nie

* | ``T fetch_add(T arg,`` 
  |               ``std::memory_order order = std::memory_order_seq_cst )``
  
  atomowo dodaje argument do wartości przechowywanej w obiekcie i zwraca poprzednią wartość
    
* | ``T fetch_add(T arg,`` 
  |               ``std::memory_order order = std::memory_order_seq_cst )``
  
  atomowo odejmuje argument od wartości przechowywanej w obiekcie i zwraca poprzednią wartość


.. .. code-block:: cpp

..     template<typename T>
..     class stack
..     {
..         std::atomic<node<T>*> head;
..      public:
..         void push(const T& data)
..         {
..             node<T>* new_node = new node<T>(data);

..           // put the current value of head into new_node->next
..           new_node->next = head.load(std::memory_order_relaxed);

..           // now make new_node the new head, but if the head
..           // is no longer what's stored in new_node->next
..           // (some other thread must have inserted a node just now)
..           // then put that new head into new_node->next and try again
..           while(!head.compare_exchange_weak(new_node->next, new_node,
..                                             std::memory_order_release,
..                                             std::memory_order_relaxed))



Opcje operacji dostępu do pamięci
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Model pamięci w C++ definiuje w jaki sposób operacje wykonywane na typach atomowych wpływają na ograniczenia dotyczące zmiany kolejności wykonywania operacji dostępu do pamięci (zapisu i odczytu)
przez kompilator lub hardware. Opcje te są definiowane poprzez wyliczenie typu ``std::memory_order``, które jest przekazywane jako argument przy wywołaniu operacji na zmiennej atomowej.

* **Sequential consistency** (``std::memory_order_seq_cst``) - ta opcja narzuca największe ograniczenia
  dotyczące uporządkowania operacji dostępu do pamięci. Wymuszona jest spójność sekwencyjna, tzn. wszystkie wątki muszą zobaczyć wszystkie operacje 
  synchronizacji w programie, które są sekwencyjnie spójne (*sequentialy consistent*) w ściśle określonej kolejności. 
  Operacje dostępu do pamięci poprzedzajęce operację oznaczoną jako ``memory_order_seq_cst`` nie mogą być przeniesione 
  poniżej punktu synchronizacji. Z kolei operacje dostępu do pamięci występujące poniżej operacji ``memory_order_seq_cst`` 
  nie mogą być przeniesione powyżej punktu synchronizacji.

* **Acquire-Release** - zapis synchronizuje się z odczytem, ale nie ma gwarancji globalnego uporządkowania operacji synchronizacji 
  (różne wątki mogą zaobserwować różną kolejność operacji synchronizacji).
  
  Dostępne są następujące opcje:
  
  - ``std::memory_order_release`` - gwarantuje ochronę przed przeniesieniem poprzedzających operacji dostępu do pamięci poniżej punktu synchronizacji
  - ``std::memory_order_acquire`` - gwarantuje ochronę przed przeniesieniem późniejszyc operacji dostępu do pamięci powyżej punktu synchronizacji
  - ``std::memory_order_consume`` - luźniejsza wersja operacji ``acquire`` - dotyczy tylko operacji, które są obliczeniowo zależne od odczytanej wartości zmiennej atomowej
  - ``std::memory_order_acq_rel`` - połączenie ograniczeń ``acquire`` i ``release`` - w miejscu synchronizacji z tą opcją wstawiana jest pełna bariera
  
  
* **Relaxed** (``std::memory_order_relaxed``) - w tym modelu jest zapewniona jedynie niepodzielność wykonywanej operacj. Nie istnieje relacja synchronizacji store-load, a co za tym idzie nie ma relacji *happens-before*  

+-----------------+-----------------------------+
| Typ operacji    | Opcja synchronizacji        |
+=================+=============================+
| | store         |  | ``memory_order_seq_cst`` |
| |               |  | ``memory_order_release`` |
| |               |  | ``memory_order_relaxed`` |
+-----------------+-----------------------------+
| | load          |  | ``memory_order_seq_cst`` |
| |               |  | ``memory_order_acquire`` |
| |               |  | ``memory_order_consume`` |
|                 |  | ``memory_order_relaxed`` |
+-----------------+-----------------------------+
| read-write-read |  | wszystkie opcje          |
+-----------------+-----------------------------+


Przykłady wykorzystania typów atomowych
=======================================

Licznik zdarzeń (*event counter*)
---------------------------------

Zmienna ``count`` jest typem atomowym, zainicjowanym zerem:

.. code-block:: c++

    std::atomic<int> count{0};


* Wątki 1..N:

  .. code-block:: c++

      while(/* ... */)
      {
        // ...
        if (/* ... */)
           count.fetch_add(1, std::memory_order_relaxed);
        // ...
      }

* Wątek główny:

  .. code-block:: c++ 

      int main()
      {
          launch_workers();
          // ...

          join_workers();  // thread exit happens-before returning from a join

          std::cout << count.load(std::memory_order_relaxed) << std::endl;
      }
  
Operacje na zmiennej atomowej ``count`` mogą mieć status ``memory_order_relaxed`` ponieważ nie występuje komunikacja
między wątkami. 


Licznik referencji (*reference counting*)
-----------------------------------------

Klasa licznika:

.. code-block:: c++

    // thread-safe counter
    template <typename T>
    class RefCounted : boost::noncopyable
    {
        std::atomic<int> ref_count_{0};

    public:
        RefCounted() = default;

        void add_ref()
        {
            ref_count_.fetch_add(1, std::memory_order_relaxed);
        }

        void release()
        {
            if (ref_count_.fetch_sub(1, std::memory_order_acq_rel) == 1)
            {
                delete static_cast<T*>(this);
            }
        }
    };

W operacji ``add_ref()`` inkrementacja może zostać zaimplementowana z tagiem ``memory_order_relaxed``, ponieważ
nie publikuje ona danych dla innych wątków.

W operacji ``release()`` dekrementacja musi być zaimplementowana przynajmniej z tagiem ``memory_order_acq_rel``.


Double-Checked Locking Pattern
------------------------------

Implementacja leniwej inicjalizacji:

.. code-block:: c++

    std::atomic<Gadget*> instance{nullptr};
    std::mutex mtx_instance;

    Gadget& get_instance()
    {
        Gadget* tmp = instance.load(std::memory_order_acquire);
        
        if (tmp == nullptr) // 1-st check
        {
            std::lock_guard<std::mutex> lk{mtx_instance};

            tmp = instance.load(std::memory_order_relaxed);

            if (tmp == nullptr) // 2-nd check
            {
                tmp = new Gadget();
                instance.store(tmp, std::memory_order_release);
            }          
        }

        return tmp;
    }