**********************
Funkcje asynchroniczne
**********************

.. epigraph::

   Future: "[...] an object that acts as a proxy for a result that is initially not known."

   -- Baker and Hewitt, 1977


Obiekt ``future<T>`` obsługuje możliwość odczytania przyszłych wartości typu ``T`` w danym wątku, które mogą być wyliczane
asynchronicznie w osobnych wątkach lub synchronicznie w tym samym wątku.
Obiekty future mogą pełnić rolę wysokopoziomowych konstrukcji synchronizujących pracę wielu wątków, zastępując przy tym
niskopoziomowe konstrukcje takie jak np. zmienne warunkowe.
Obiektów *future* można używać nie tylko w programowaniu wielowątkowym, ale także w programach jednowątkowych.

Do obsługi wartości typu *future* wykorzystywane są cztery klasy:

* ``std::future<T>``
* ``std::shared_future<T>``
* ``std::promise<T>``
* ``std::packaged_task<T>``
* oraz funkcja ``std::async()``


Futures
=======

Obiekty typu futures umożliwiają odczytanie wyników funkcji asynchronicznych - ``std::future<T>``
oraz ``std::shared_future<T>``.

.. cpp:class:: std::future<typename T>

    Obiekt typu ``std::future<T>`` reprezentuje unikalna wartość typu *future* (unikalne prawo własności do przyszłego
    wyniku wywołania funkcji).

.. cpp:class:: std::shared_future<typename T>

    Implementacja współdzielonych wartości typu *future*. Może być użyta do sygnalizacji między wieloma wątkami, które
    mogą współdzielić obiekt *future*.

Możliwa jest konwersja obiektu ``future<T>`` do współdzielonego obiektu ``shared_future<T>`` przy pomocy:

* konstruktora przenoszącego ``std::shared_future<T>(std::future&& other)``
* metody ``std::future<T>::share()`` zwracającej obiekt typu ``std::shared_future<T>``

Metody:

.. cpp:function:: T get()

    wstrzymuje bieżący wątek do momentu zakończenia asynchronicznej funkcji i następnie zwraca otrzymaną wartość lub
    rzuca przechowany w *shared state* wyjątek

    .. code-block:: c++

        #include <iostream>
        #include <future>
        #include <thread>
         
        int fib(int n)
        {
            if (n < 3) return 1;
            else return fib(n-1) + fib(n-2);
        }
         
        int main()
        {
            std::future<int> f1 = std::async(std::launch::async, [] { return fib(20); });
            std::future<int> f2 = std::async(std::launch::async, [] { return fib(25); });
         
            std::cout << "waiting...\n";
         
            std::cout << "f1: " << f1.get() << '\n';
            std::cout << "f2: " << f2.get() << '\n';
        }


.. cpp:function:: void wait() const

    blokuje bieżący wątek dopóki wynik nie zostanie obliczony

.. cpp:function:: std::future_status wait_for(const std::chrono::duration<Rep, Period>& timeout_duration) const

    czeka przez określony interwał czasu aż wynik zostanie obliczony. Zwracany jest status, który może przyjąć jedną z
    trzech wartości:

    - ``future_status::deferred`` - funkcja nie została jeszcze uruchomiona
    - ``future_status::ready`` - wynik obliczenia jest gotowy
    - ``future_status::timeout`` - czas oczekiwania na wynik się zakończył (funkcja jeszcze nie zwróciła wyniku)

    .. code-block:: c++
  
        #include <iostream>
        #include <future>
        #include <thread>
        #include <chrono>
     
        int main()
        {
            std::future<int> future = std::async(std::launch::async, [](){ 
                std::this_thread::sleep_for(std::chrono::seconds(3));
                return 8;  
            }); 
     
            std::cout << "waiting...\n";
            std::future_status status;
            do 
            {
                status = future.wait_for(std::chrono::seconds(1));

                if (status == std::future_status::deferred) 
                {
                    std::cout << "deferred\n";
                } 
                else if (status == std::future_status::timeout) 
                {
                    std::cout << "timeout\n";
                } 
                else if (status == std::future_status::ready) 
                {
                    std::cout << "ready!\n";
                }
            } while (status != std::future_status::ready); 
         
            std::cout << "result is " << future.get() << '\n';
        }


Wywołania asynchroniczne za pomocą funkcji ``std::async()``
===========================================================

Funkcja szablonowa ``async()`` umożliwia asynchroniczne uruchomienie obiektu wywoływalnego *Callable* (potencjalnie w
osobnym wątku). Zwracana jest instancja ``std::future``, która przechowuje przyszły wynik wywołania funkcji.

* .. cpp:function:: template<class Function, class... Args> \
                    auto async(std::launch policy, Function&& f, Args&&... args)

    Wywołuje funkcję ``f`` przekazując do niej argumenty ``args`` zgodnie wybraną polityką uruchomienia:

    * ``std::launch::async``- wywołanie w osobnym wątku (wywołanie asynchroniczne)

    * ``std::lauch::deferred`` - nie tworzy osobnego wątku. Leniwie wykonuje funkcję ``f`` (wywołanie następuje
      w momencie pierwszego wywołania na obiekcie *future* metod ``get()`` lub ``wait()``


* .. cpp:function:: template<class Function, class... Args> \
                    auto async(Function&& f, Args&&... args)

    Zachowuje się tak samo jak ``async(std::launch::async | std::launch::deferred, f, args...)``, tzn. funkcja ``f``
    może zostać wykonana w osobnym wątku lub synchronicznie w wątku bieżącym, gdy obiekt *future* zostanie odpytany o
    wynik operacji. Od wersji C++14 ta wersja funkcji ``async()`` jest uznana za *deprecated*.


Klasa ``std::promise``
======================

.. cpp:class:: template <typename T> \
               std::promise<T>

    Implementuje niskopoziomowe mechanizmy przechowanie wartości lub wyjątku, które później mogą być odczytane
    asynchronicznie poprzez obiekt ``std::future<T>`` wygenerowany przez instancję ``std::promise<T>``.

    Obiekt ``promise<T>`` jest związany ze **współdzielonym stanem** (*shared state*), który przechowuje pewną informację o
    stanie oraz wynik, który może być jeszcze nie obliczony, obliczony do wartości lub obliczony do wyjątku.

    Obiekt ``promise<T>`` może zrobić trzy rzeczy ze stanem współdzielonym:

    * oznaczyć stan współdzielony jako gotowy do  odczytu - ``promise<T>`` zapisuje wartość ``set_value(T)``
      lub wyjątek ``set_exception(T)``

    * zwolnić go - instancja ``promise<T>`` zwalnia referencję do stanu współdzielonego. Jeśli to była ostatnia
      referencja do stanu współdzielonego, stan ten jest niszczony. Operacja ta nie jest blokująca, za wyjątkiem
      sytuacji, gdy stan współdzielony został utworzony przy pomocy funkcji ``std::async()``

    * porzucić go - ustawiając jako wyjątek instancję typu ``std::future_error`` z kodem błędu
      ``std::future_error::broken_promise``


Każdy obiekt ``std::promise<T>`` związany jest jednym obiektem ``std::future<T>``.

Wątek z dostępem do obiektu *future* po wywołaniu metody ``wait()`` będzie czekać na rezultat zwrócony przez obiekt
``std::promise`` z innego wątku przy pomocy metody ``set_value()``.

.. code-block:: c++

    #include <thread>
    #include <future>
    #include <chrono>
    #include <iostream>

    std::string load_file_content()
    {
        std::string content = "FILE content";

        using namespace std::chrono;

        if (duration_cast<milliseconds>(
                steady_clock::now().time_since_epoch()).count() % 2 == 0)
            throw std::runtime_error("I/O error");

        return content;
    }


    class BackgroundTask
    {
        std::promise<std::string> promise_;
    public:
        void operator()()
        {
            std::this_thread::sleep_for(std::chrono::seconds(3));

            try
            {
                std::string result = load_file_content();

                promise_.set_value(result);
            }
            catch (...)
            {
                promise_.set_exception(std::current_exception());
            }
        }

        std::future<std::string> get_future()
        {
            return promise_.get_future();
        }
    };

    void waiter(std::future<std::string> f)
    {
        try
        {
            std::cout << f.get() << std::endl;
        }
        catch(const std::runtime_error& e)
        {
            std::cout << "Exception caught: " << e.what() << std::endl;
        }
    }

    int main()
    {
        BackgroundTask bt;
        std::future<std::string> future = bt.get_future();

        std::thread thd1(waiter, std::move(future));
        std::thread thd2(std::ref(bt));
        thd1.join();
        thd2.join();
    }


Klasa ``std::packaged_task``
============================

`std::packaged_task<R(Args...)>`
: 

Klasa ``std::packaged_task<T>`` jest pomocniczym obiektem opakowującym wywołanie funkcji lub funktora (obiektu
wywoływalnego - *callable*) i implementującym zapisanie wyniku w *shared state*, który może być odczytany przez obiekt
``std::future<T>``.

.. code-block:: c++

    #include <future>

    // definicja zadania
    std::packaged_task<int (int)> pt([](int i) { return i * i; });

    // wywołanie zadania
    pt(17);

    // odczytanie wyniku
    auto result = pt.get_future().get();

``packaged_task`` pozwala oddzielić:

* **definicje zadania**, które ma być wykonane
* **wywołanie** zadania, które może zostać wykonane w osobnym wątku
* **przetworzenie wyników** zadania

Przykład wykorzystania obiektów ``packaged_task``:

.. code-block:: c++

    #include <iostream>
    #include <thread>
    #include <future>
    #include <vector>

    using namespace std;

    void run_all_tasks(vector<packaged_task<int (char)>>& tasks)
    {
        int i = 0;

        // iterate over tasks and start them with parameters 1, 2, 3...
        for(auto& task : tasks)
        {
            char c = '1' + i++;
            task(c); // run packaged_task with argument c
        }
    }

    int func(char c)
    {
        for(int i = 0; i < 10; ++i)
        {
            cout.put(c).flush();
            this_thread::sleep_for(chrono::milliseconds(300));
        }

        return c;
    }

    int main()
    {

        vector<packaged_task<int (char)>> tasks;

        // definition of some tasks
        packaged_task<int (char)> t1(&func);
        tasks.push_back(move(t1));

        packaged_task<int (char)> t2([](char c) { return c; });
        tasks.push_back(move(t2));

        tasks.emplace_back(&func);


        // start all tasks in one separate thread
        auto future_all_tasks = async(launch::async, &run_all_tasks, ref(tasks));

        // wait until all tasks are done
        future_all_tasks.wait();

        // process results of each task
        for(auto& task : tasks)
        {
            cout << "result: " << task.get_future().get() << endl;
        }
    }


Obiekt ``packaged_task`` może zostać użyty w połączeniu z obiektami ``shared_future`` do implementacji funkcji
asynchronicznych.

.. code-block:: c++

    #include <iostream>
    #include <string>
    #include <thread>
    #include <future>
    #include <chrono>

    using namespace std::literals;

    typedef std::string FileContent;

    FileContent download_file(const std::string& url)
    {
        std::this_thread::sleep_for(2s);

        return "content of file: " + url;
    }

    std::shared_future<FileContent> download_file_async(const std::string& url)
    {
        std::packaged_task<FileContent ()> task([=] { return download_file(url); });
        std::shared_future<FileContent> sf(task.get_future());
        std::thread thd(move(task));
        thd.detach();
        return sf;
    }

    void process(std::shared_future<FileContent> file_content)
    {
        std::cout << "Processing " << file_content.get() << " in a thread#" << this_thread::get_id() << std::endl;

    }

    int main()
    {
        std::shared_future<FileContent> content =
            download_file_async("http://infotraining.pl");

        std::thread thd1{&process, content};
        std::thread thd2{ [content]() -> void { 
            cout << "Processed " << content.get() 
                 << " by lambda in a thread#" << this_thread::get_id() << endl; } 
        };

        thd1.join();
        thd2.join();
    }
