#include "common.hpp"

#include <chrono>
#include <iostream>
#include <stop_token>
#include <string>
#include <thread>

// #region safe_join
void background_work(const int id, const std::string text,
    const std::chrono::milliseconds delay)
{
    std::cout << "Thread#" << id << " started..." << std::endl;

    for (const auto& letter : text)
    {
        std::cout << "Thread#" << id << " - " << letter << std::endl;
        std::this_thread::sleep_for(delay);
    }
}

void safe_join_in_cpp20()
{
    std::jthread thd_1{&background_work, 1, "Hello", 100ms};
    std::jthread thd_2{&background_work, 2, "Multithreading", 250ms};

    may_throw(); // if it throws, both threads are joined in the destructors

    thd_1.join(); // explicit join
} // implicit join of thd_2 in the destructor
// #endregion

// #region stop_token_task
void cancellable_work(std::stop_token st, const int id, const std::string text)
{
    std::cout << "Thread#" << id << " started..." << std::endl;

    for (const auto& letter : text)
    {
        if (st.stop_requested())
        {
            std::cout << "Stop has been requested..." << std::endl;
            std::cout << "Thread#" << id << " cancelled" << std::endl;
            return;
        }

        std::cout << "Thread#" << id << " - " << letter << std::endl;
        std::this_thread::sleep_for(250ms);
    }
}
// #endregion

// #region run_task_and_cancel
void run_task_and_cancel()
{
    std::stop_source stop_src;
    std::stop_token st = stop_src.get_token();

    std::jthread thd_1{&cancellable_work, st, 1, "FIRST_TASK"};
    std::jthread thd_2{&cancellable_work, st, 2, "SECOND_TASK"};

    std::this_thread::sleep_for(2s);

    stop_src.request_stop();

    // ...
}
// #endregion

// #region internal_stop_state
void use_internal_stop_state()
{
    std::jthread thd_1{&cancellable_work, 1, "TASK"}; // no stop-token passed

    std::stop_source stop_src = thd_1.get_stop_source(); // get stop_source from jthread
    std::stop_token st = thd_1.get_stop_token();

    stop_src.request_stop(); // or thd1.request_stop();
}
// #endregion

//#region stop_callback
void stop_callback_example()
{
    std::jthread thd{&cancellable_work, 1, "TASK"};
    std::stop_token st = thd.get_stop_token();

    std::stop_callback cb_1(st, []{
        std::cout << "Stop requested callback#1 invoked from " 
            << std::this_thread::get_id() << std::endl;
    });

    std::stop_callback cb_2(st, []{
        std::cout << "Stop requested callback#2 invoked from " 
        << std::this_thread::get_id() << std::endl;
    });

    std::thread thd_stop_requester([&thd]{
        std::this_thread::sleep_for(1s);
        thd.request_stop();
    });
    
    thd_stop_requester.join();
}
//#endregion

int main()
{
    safe_join_in_cpp20();
    run_task_and_cancel();
    use_internal_stop_state();
    stop_callback_example();
}
