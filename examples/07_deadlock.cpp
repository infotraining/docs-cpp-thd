#include "common.hpp"

#include <iostream>
#include <mutex>
#include <thread>

// #region deadlock
class XDeadlock
{
    mutable std::mutex mtx_;
    int data_;

public:
    XDeadlock(int data)
        : data_{data}
    {
    }

    bool operator<(const XDeadlock& other) const
    {
        std::lock_guard<std::mutex> lk1(mtx_);
        std::lock_guard<std::mutex> lk2(other.mtx_); // possible deadlock
        return data_ < other.data_;
    }
};
// #endregion

// #region std_lock
class XStdLock
{
    mutable std::mutex mtx_;
    int data_;

public:
    XStdLock(int data)
        : data_{data}
    {}

    bool operator<(const XStdLock& other) const
    {
        std::unique_lock<std::mutex> l1(mtx_, std::defer_lock);
        std::unique_lock<std::mutex> l2(other.mtx_, std::defer_lock);

        std::lock(l1, l2); // avoiding deadlock
        return data_ < other.data_;
    }
};
// #endregion

// #region std_lock_cpp17
class XStdLockCpp17
{
    mutable std::mutex mtx_;
    int data_;

public:
    XStdLockCpp17(int data)
        : data_{data}
    {
    }

    bool operator<(const XStdLockCpp17& other) const
    {
        std::unique_lock l1(mtx_, std::defer_lock);
        std::unique_lock l2(other.mtx_, std::defer_lock);

        std::lock(l1, l2); // avoiding deadlock
        return data_ < other.data_;
    }
};
// #endregion

// #region scoped_lock
class XScopedLock
{
    mutable std::mutex mtx_;
    int data_;

public:
    XScopedLock(int data)
        : data_{data}
    {
    }

    bool operator<(const XScopedLock& other) const
    {
        std::scoped_lock lk{mtx_, other.mtx_};
        return data_ < other.data_;
    }
};
// #endregion

template <typename X>
void compare_in_both_directions()
{
    X x1{1};
    X x2{2};

    std::jthread thd1{[&] { for (int i = 0; i < 10'000; ++i) (void)(x1 < x2); }};
    std::jthread thd2{[&] { for (int i = 0; i < 10'000; ++i) (void)(x2 < x1); }};
}

int main()
{
    // XDeadlock is intentionally not run - it may deadlock
    compare_in_both_directions<XStdLock>();
    compare_in_both_directions<XStdLockCpp17>();
    compare_in_both_directions<XScopedLock>();

    std::cout << "No deadlock" << std::endl;
}
