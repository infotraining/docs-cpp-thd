#include "common.hpp"

#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

// #region lock_guard
template <typename Value>
struct Synchronized
{
    Value value;
    std::mutex mtx_value;
};

template <typename F, typename Value>
void apply(F f, Synchronized<Value>& sync_value)
{
    std::lock_guard<std::mutex> lk{sync_value.mtx_value};
    f(sync_value.value);
}
// #endregion

// #region lock_guard_usage
void use_synchronized()
{
    Synchronized<std::vector<int>> sync_vec;

    std::thread thd([&] {
        apply([](std::vector<int>& v) {
            v.push_back(1);
            v.push_back(2);
        }, sync_vec);
    });
    thd.join();
}
// #endregion

// #region timed_mutex
void try_lock_with_timeout(std::timed_mutex& mtx)
{
    std::unique_lock lk(mtx, std::try_to_lock);

    if (!lk.owns_lock())
    {
        do
        {
            std::cout << "Thread does not own a lock..."
                      << " Tries to acquire a mutex..."
                      << std::endl;
        } while (!lk.try_lock_for(1s));
    }

    std::cout << "Lock acquired" << std::endl;
}
// #endregion

int main()
{
    use_synchronized();

    std::timed_mutex mtx;
    std::jthread holder{[&mtx] {
        std::lock_guard lk{mtx};
        std::this_thread::sleep_for(2500ms);
    }};
    std::this_thread::sleep_for(100ms);

    try_lock_with_timeout(mtx);
}
