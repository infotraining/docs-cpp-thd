*****
Wątki
*****

Zarządzanie wątkami
===================

* Klasa ``std::thread`` jest odpowiedzialna za tworzenie obiektów, które uruchamiają wątki i zarządzają nimi
* Każdy obiekt ``std::thread`` reprezentuje

  - pojedynczy wątek, utworzony przez system operacyjny 
  - lub tzw. wątek pusty (*Not-A-Thread*)
  
    + jest to instancja ``std::thread`` utworzona konstruktorem domyślnym
    + lub obiekt wątku po wykonaniu na nim operacji ``std::move()``
      
* Obiekty wątków nie są kopiowalne (*noncopyable*)
* Wątki mogą być jednak przenoszone między obiektami - semantyka przenoszenia (*move semantics*)

  - do przenoszenia wątków, które są obiektami *l-value* należy użyć 
    funkcji ``std::move()``


Tworzenie i uruchamianie wątków
===============================

Wątek jest uruchamiany przez przekazanie w konstruktorze obiektu ``std::thread``:


Wskaźnika do funkcji
--------------------

.. code-block:: c++

    #include <thread>

    void my_task()
    {
        std::cout << "My first thread..." << std::endl;
    }

    int main()
    {
        std::thread thd(&my_task);
        thd.join();
    }


Obiektu funkcyjnego
-------------------

.. code-block:: c++

    #include <thread>

    class BackgroundTask
    {
    public:
        void operator()() const
        {
            std::cout << "Hello from a thread..." << std::endl;
        }
    };

    int main()
    {
        BackgroundTask bt;
        std::thread t1(bt);
        std::thread t2(BackgroundTask());
        t1.join();
        t2.join();
    }


Funkcji lambda
--------------

.. code-block:: c++

    #include <thread>

    int main()
    {
        std::thread t([] { std::cout << "My first thread..." << std::endl; });
        t.join();
    }


Dołączenie do wątku
===================

Aby poczekać na zakończenie wykonywania zadania przez dany wątek należy wywołać na jego rzecz metodę ``join()``.
Metoda ``join()`` wstrzymuje wykonanie bieżącego wątku, aż do czasu zakończenia pracy przez wskazany wywołaniem wątek.

.. code-block:: c++

    int main()
    {
        BackgroundTask bt;

        std::thread t(bt);
        t.join(); // wstrzymanie wykonania funkcji main,
                  // aż do czasu zakończenia wątku t

        assert(!thd.joinable());
    }


Odłączanie wątków od obiektów
=============================

Wątki odłączone od obiektu:

* wątki tła
* demony

Aby utworzyć wątek tła należy na rzecz obiektu reprezentującego dany wątek wywołać metodę ``detach()``

.. code-block:: c++

    std::thread thd(&do_background_work);

    thd.detach();

    assert(!thd.joinable());

Zgodnie z wytycznymi `C++ Core Guidelines [CP.26] <https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#cp26-dont-detach-a-thread>`_
nie jest zalecane odłączanie wątków za pomocą ``detach()``. 


Przekazywanie parametrów do wątków
==================================

Przekazanie parametrów do uruchamianych wątków może odbywać się na trzy sposoby:

* Korzystając z parametrów konstruktora obiektu funkcyjnego przekazywanego do konstruktora obiektu ``std::thread``

  .. code-block:: c++

        class BackgroundTask
        {
            int x_;
            double y_;

        public:
            BackgroundTask(int x, double y) : x_(x), y_(y) {}

            void operator()()
            {
                //... implementacja wykorzystuje składowe x_ i y_
            }
        };

        //...
        BackgroundTask bt(1, 3.14);
        std::thread thd(bt);
        thd.join();


* Przekazując argumenty jako kolejne (po funkcji lub obiekcie funkcyjnym) parametry konstruktora ``std::thread``. Podane argumenty są forwardowane (kopiowane lub przenoszone) do uruchamianego wątku.
  
  Jeśli wymagane jest przekazanie referencji do funkcji uruchamianej w nowym wątku należy użyć standardowych wraperów referencji ``std::ref()`` lub ``std::cref()``.

  .. code-block:: c++
 
    void f1(int n)
    {
        for (int i = 0; i < n; ++i) 
        {
            std::cout << "Thread 1 executing\n";
            ++n; // increment of local copy
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
     
    void f2(int& n)
    {
        for (int i = 0; i < 5; ++i) 
        {
            std::cout << "Thread 2 executing\n";
            ++n;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
     
    int main()
    {
        int n = 0;
        std::thread thd1(f1, n + 1); // pass by value
        std::thread thd2(f2, std::ref(n)); // pass by reference
        thd1.join();
        thd2.join();
        std::cout << "Final value of n is " << n << '\n';
    }


* Wykorzystując obiekt domknięcia, który przechwytuje zmienne będące argumentami wywoływanej funkcji

  .. code-block:: c++

        void process(int index, std::vector<int>& data)
        {
            //... processing data
            
            data[index] = calculated_value;
        }

        //...

        std::vector<int> vec(2);

        std::thread thd1{proces, 0, std::ref(vec)};
        std::thread thd2{[&vec] { process(1, vec); }};

        thd1.join();
        thd2.join();


Problem wiszących referencji
----------------------------

Przy przekazywaniu parametrów do wątków należy unikać wiszących 
referencji (*dangling references*) - `C++ Core Guidelines [CP.31] <https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#cp31-pass-small-amounts-of-data-between-threads-by-value-rather-than-by-reference-or-pointer>`_ .


.. code-block:: c++

    class Worker
    {
        int& ref;

    public:
        Worker(int& r) : ref(r) {}

        void operator()()
        {
            do_something(i);
        }
    };

    std::thread create_thread()
    {
        int local_value = 0;
        Worker w(local_value);

        std::thread thd(w);
        return thd;
    } // local_value jest niszczone – referencja do tej zmiennej traci ważność


Transferowanie wątków
=====================

Obiekty wątków mogą przenoszone zgodnie z zasadami **move semantics**. Aby przenosić wątki między obiektami należy skorzystać z funkcji ``std::move()``

.. code-block:: c++

    void task()
    {
        /* implementation */
    }

    std::thread create_thread()
    {
        std::thread thd(&task);
        return thd;
    }

    std::thread threads[10];

    std::thread t(&task);
    threads[5] = create_thread();
    threads[6] = std::move(t);


Destruktor wątku
================

Destrukcja obiektu wątku jest bezpieczna, jeśli obiekt wątku nie jest skojarzony z wątkiem systemowym.
W przeciwnym wypadku wywołana jest funkcja ``std::terminate()``!!!

Obiekt wątku nie jest skojarzony z wątkiem systemowym:

* jeśli został utworzony za pomocą konstruktora domyślnego
* został przeniesiony do innego obiektu - np. jest po wywołaniu ``std::move()``
* została wcześniej wywołana operacja ``join()``
* została wcześniej wywołana operacja ``detach()``

Jeśli obiekt wątku jest skojarzony z wątkiem systemowym wywołanie metody ``joinable()`` zwraca ``true``.

Kontrola wątku przy pomocy RAII
-------------------------------

Standard C++ zmusza użytkownika wątku do jawnego określenia czy staje się on wątkiem tła, czy też oczekiwane jest zakończenie wykonywania uruchomionego wątku.
Jawne wykonanie operacji na wątku, które jest wymagane przed wywołaniem destruktora obiektu może w obecności wyjątków doprowadzić do zakończenia wykonywania programu.
Sytuacja ta powinna zostać rozwiązana przy pomocy obiektu implementującego technikę RAII. Niestety biblioteka standardowa do tej pory nie posiada odpowiedniej implementacji.

Przykładowa implementacja klasy kontrolującej wyjątek poprzez wywołanie operacji ``join()`` w destruktorze:

.. code-block:: c++

    template <typename...>
    struct Typelist;

    class JoiningThread
    {
        thread thd_;

    public:
        // perfect forwarding constructor
        template <
            typename... Args,
            typename = 
                std::enable_if_t<
                    !is_same<Typelist<JoiningThread>, 
                             Typelist<std::decay_t<Args>...>>::value>>
        JoiningThread(Args&&... args) : thd_{forward<Args>(args)...}
        {
        }

        JoiningThread(const JoiningThread&) = delete;
        JoiningThread& operator=(const JoiningThread&) = delete;

        JoiningThread(JoiningThread&&) = default;
        JoiningThread& operator=(JoiningThread&&) = default;

        ~JoiningThread()
        {
            if (thd_.joinable())
                thd_.join();
        }

        thread& get()
        {
            return thd_;
        }
    };

Użycie klasy ``JoiningThread``:

.. code-block:: c++

    unique_ptr<JoiningThread> deamon{nullptr};

    void local_scope2()
    {        
        JoiningThread raii_thd1{&background_work, 3, 100ms};
        JoiningThread raii_thd2{thread{&background_work, 4, 100ms}};

        //JoiningThread illegal_copy = raii_thd1; // compiler error
        JoiningThread moved_raii_thd = move(raii_thd1);
        assert(raii_thd1.get().get_id() == thread::id{});

        vector<JoiningThread> threads;
        threads.push_back(move(moved_raii_thd));
        threads.push_back(move(raii_thd2));
        threads.emplace_back(&background_work, 5, 400ms);

        // creating a deamon
        deamon = make_unique<JoiningThread>(&heartbeat, 800ms);

        may_throw();
    }    


Kontenery wątków
================

* Obiekty wątków mogą być przechowywane w kontenerach standardowych

.. code-block:: c++

    void task() {...}  // funkcja wątku

    std::vector<std::thread> threads;

    std::thread thd1(&task);
    threads.push_back(std::move(thd1));
    threads.push_back(std::thread(&task));
    threads.emplace_back(&task);

    for(auto& t : threads)
        t.join();


Obsługa wyjątków w wątkach
==========================

Jeżeli z funkcji uruchomionej w osobnym wątku wydostanie się wyjątek, zostanie wywołana funkcja ``std::terminate()``.
W celu prawidłowej obsługi wyjątków, należy przechwycić rzucony wyjątek i jeżeli istnieje taka potrzeba przekazać go
do wątku rodzica wykorzystując klasę ``std::exception_ptr``. 
Umożliwia ona przechowanie wskaźnika do wyjątku, który został zgłoszony instrukcją ``throw`` i przechwycony 
funkcją ``std::current_exception()``. Instancja ``std::exception_ptr`` może być przekazana do innej funkcji, również 
takiej, która uruchomiona jest w osobnym wątku. Obsługa przekazanego przez wskaźnik wyjątku jest możliwa
przy pomocy funkcji ``std::rethrow_exception()``, która powoduje ponowne rzucenie wyjątku.

* Domyślnie skonstruowany obiekt ``std::exception_ptr`` ma wartość ``nullptr``. 
* Dwa obiekty są uznawane za równe, jeśli są puste lub wskazują na ten sam obiekt wyjątku.
* Instancja ``std::exception_ptr`` jest konwertowalna do wartości logicznej

.. code-block:: c++

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

    // ...

    int main()
    {
        std::exception_ptr thd_exception;

        std::thread thd(&background_work, std::ref(thd_exception));

        //...

        thd.join();

        try
        {
            std::rethrow_exception(e);
        }
        catch(const std::runtime_error& e)
        {
            // ... handling an error
        }
    }


Funkcje i klasy pomocnicze w bibliotece standardowej
====================================================


``std::thread::hardware_concurrency()``
---------------------------------------

Statyczna metoda zwracająca ilość dostępnych wątków sprzętowych.
Zwykle podawana jest ilość procesorów, rdzeni, itp.
Jeżeli informacja nie jest dostępna zwraca wartość ``0``.

.. code-block:: c++

    cout << "hardware_concurrency() = "
        << thread::hardware_concurrency() << endl;

W praktyce często używa kontenerów wątków o rozmiarze równym ilości wątków sprzętowych:

.. code-block:: c++

    auto hardware_threads_count = std::max(1u, std::thread::hardware_concurrency());
    std::vector<std::thread> threads(hardware_threads_count);



Przestrzeń nazw ``std::this_thread``
------------------------------------

Przestrzeń nazw ``std::this_thread`` zawiera zestaw pomocniczych funkcji lub klas:

* ``sleep_for(const chrono::duration<Rep, Period>& sleep_duration)`` - wstrzymuje 
  wykonanie bieżącego wątku na (przynajmniej) określony interwał czasu

  .. code-block:: c++
  
      using namespace std::chrono_literals;
      std::cout << "Hello waiter" << std::endl;
      auto start = std::chrono::high_resolution_clock::now();
      std::this_thread::sleep_for(2s);
      auto end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double, std::milli> elapsed = end-start;
      std::cout << "Waited " << elapsed.count() << " ms\n";

* ``sleep_until(const chrono::time_point<Clock, Duration>& sleep_time)`` - blokuje 
  wykonanie wątku przynajmniej do podanego jako parametr punktu czasu
  
* ``yield()`` - funkcja umożliwiające podjęcie próby wywłaszczenia bieżącego wątku
  i przydzielenia czasu procesora innemu wątkowi

* ``get_id()`` - zwraca obiekt typu ``std::thread::id`` reprezentujący identyfikator
  bieżącego wątku


Identyfikator wątku 
-------------------

Klasa ``std::thread::id`` jest lekką, trywialnie kopiowalną klasą, która opakowuje unikalny identyfikator wątku. Celem klasy jest umożliwienie wykorzystania identyfikatora
wątku jako klucza w kontenerach asocjacyjnych (np. ``std::map``, ``std::unordered_map``).

.. code-block:: c++

    class thread::id
    {
    public:
        id();
        bool operator==(const id& y) const;
        bool operator!=(const id& y) const;
        bool operator<(const id& y) const;
        bool operator>(const id& y) const;
        bool operator<=(const id& y) const;
        bool operator>=(const id& y) const;
        
        template<class charT, class traits>
        friend std::basic_ostream<charT, traits>&
            operator<<(std::basic_ostream<charT, traits>& os, const id& x);
    };


- Domyślny konstruktor tworzy obiekt identyfikujący tzw. pusty wątek (*Not-A-Thread*).
- Obiekty typu ``thread::id`` są kopiowalne, porównywalne oraz hashowalne.
  
  - W klasie zdefiniowany jest pełen zestaw operatorów porównania
  - Biblioteka standardowa definiuje pomocniczą klasę ``std::hash<std::thread::id>``, która umożliwia
    przechowanie wątku w kontenerach hashujących (np. ``unordered_map``)
- Klasa posiada operator wyjścia do strumienia ``<<`` zależny od implementacji.

.. code-block:: c++

    std::thread::id master_thread;

    void some_core_part_of_algorithm()
    {
        if (std::this_thread::get_id() == master_thread)
            do_master_thread_work();

        do_common_work();
    }