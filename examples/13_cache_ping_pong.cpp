#include <atomic>
#include <chrono>
#include <iostream>
#include <new>
#include <thread>

// #region false_sharing
struct DataSharedLine
{
    std::atomic<int> v1{0};
    std::atomic<int> v2{0}; // v1 & v2 share a cache line -> cache ping-pong
};
// #endregion

// #region solution
struct Data
{
    alignas(std::hardware_destructive_interference_size) std::atomic<int> v1{0};
    alignas(std::hardware_destructive_interference_size) std::atomic<int> v2{0};
};
// #endregion

template <typename T>
auto measure()
{
    T data;
    const auto start = std::chrono::steady_clock::now();
    {
        std::jthread t1{[&] { for (int i = 0; i < 20'000'000; ++i) data.v1.fetch_add(1, std::memory_order_relaxed); }};
        std::jthread t2{[&] { for (int i = 0; i < 20'000'000; ++i) data.v2.fetch_add(1, std::memory_order_relaxed); }};
    }
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
}

int main()
{
    std::cout << "shared cache line: " << measure<DataSharedLine>().count() << " ms\n";
    std::cout << "aligned:           " << measure<Data>().count() << " ms\n";
}
