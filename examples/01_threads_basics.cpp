#include "common.hpp"

#include <cassert>
#include <iostream>
#include <thread>

void some_function() { }

// #region empty_thread
void empty_thread()
{
    std::thread thd_empty; // not-a-thread

    std::thread thd_1(&some_function);
    std::thread thd_2 = std::move(thd_1); // thd_1 is now not-a-thread
    thd_2.join();
}
// #endregion

// #region function_pointer
void background_task()
{
    std::cout << "My first thread..." << std::endl;
}

void run_function_pointer()
{
    std::thread thd(&background_task);
    thd.join();
}
// #endregion

// #region functor
class BackgroundTask {
    const int id_;

public:
    BackgroundTask(int id)
        : id_{id}
    {}

    void operator()() const
    {
        std::cout << "Background task " << id_ << " is running..." << std::endl;
    }
};

void run_function_object()
{
    BackgroundTask bt{1};
    std::thread thd_1(bt);
    std::thread thd_2(BackgroundTask{2});
    thd_1.join();
    thd_2.join();
}
// #endregion

// #region lambda
void run_lambda()
{
    auto task = [] {
        std::cout << "My first task..." << std::endl;
    };

    std::thread thd_1(task);

    std::thread thd_2([] {
        std::cout << "My second task..." << std::endl;
    });

    thd_1.join();
    thd_2.join();
}
// #endregion

// #region join
void join_thread()
{
    BackgroundTask bt{1};

    std::thread thd(bt);
    thd.join(); // blocks the current thread until the task in thd is finished

    assert(!thd.joinable());
}
// #endregion

int main()
{
    empty_thread();
    run_function_pointer();
    run_function_object();
    run_lambda();
    join_thread();
}
