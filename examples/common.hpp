// Shared helpers for the examples used in the slides.
//
// Conventions used by all examples:
//  - C++20, standard library names always prefixed with std::
//  - one example per function (or namespace), shown on the slides via
//    `<<< @/../examples/<file>.cpp#<region>`
//  - regions are delimited with `// #region name` / `// #endregion`
//  - helpers (Data, Gadget, ThreadSafeQueue, ...) live in this header
#pragma once

#include <chrono>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <string>
#include <thread>

using namespace std::literals;

struct Data
{
    int value;
};

inline Data process_data(const std::string& text)
{
    return Data{static_cast<int>(text.size())};
}

// throws std::runtime_error when fail == true
inline void may_throw(bool fail = false)
{
    if (fail)
        throw std::runtime_error("error from may_throw()");
}

// throws std::runtime_error for arg == 13
inline int calculate(int arg)
{
    if (arg == 13)
        throw std::runtime_error("unlucky number");
    return arg * arg;
}

inline std::uint64_t fibonacci(int n)
{
    std::uint64_t a = 0, b = 1;
    for (int i = 0; i < n; ++i)
    {
        const auto next = a + b;
        a = b;
        b = next;
    }
    return a;
}

class Gadget
{
    int id_;
    std::string name_;

public:
    Gadget(int id, std::string name)
        : id_{id}
        , name_{std::move(name)}
    {
    }

    int use() const
    {
        std::cout << "Using gadget " << name_ << std::endl;
        return id_;
    }
};

template <typename T>
class ThreadSafeQueue
{
    std::queue<T> queue_;
    mutable std::mutex mtx_;

public:
    void push(T item)
    {
        std::lock_guard lk{mtx_};
        queue_.push(std::move(item));
    }

    bool try_pop(T& item)
    {
        std::lock_guard lk{mtx_};
        if (queue_.empty())
            return false;
        item = std::move(queue_.front());
        queue_.pop();
        return true;
    }

    size_t size() const
    {
        std::lock_guard lk{mtx_};
        return queue_.size();
    }
};
