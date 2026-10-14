#include "common.hpp"

#include <cstdint>
#include <functional>
#include <iostream>
#include <thread>

constexpr std::uint32_t IMUL = 1103515245;
constexpr std::uint32_t IADD = 12345;
constexpr std::uint32_t MASK = 0x7FFFFFFF;
constexpr double SCALE = 1.0 / 2147483648.0;

// #region unsafe_random
namespace Unsafe 
{
    double rand() {
        static std::uint32_t seed = 665; // shared state

        seed = (seed * IMUL + IADD) & MASK;
        return seed * SCALE;
    }
}
// #endregion

// #region unsafe_random_usage
void use_unsafe_random() 
{
    std::jthread thd1([] {
        for (size_t i = 0; i < 10; ++i)
            std::cout << Unsafe::rand() << std::endl;
    });

    std::jthread thd2([] {
        for (size_t i = 0; i < 10; ++i)
            std::cout << Unsafe::rand() << std::endl;
    });
}
// #endregion

// #region thread_safe_random
namespace ThreadSafe {
    double rand_double() {
        std::hash<std::thread::id> hasher;
        thread_local std::uint32_t thd_seed =
            665 * static_cast<std::uint32_t>(hasher(std::this_thread::get_id()));

        thd_seed = (thd_seed * IMUL + IADD) & MASK;
        return thd_seed * SCALE;
    }
}
// #endregion

// #region thread_safe_random_usage
void use_thread_safe_random() {
    std::jthread thd1([] {
        for (size_t i = 0; i < 10; ++i)
            std::cout << ThreadSafe::rand_double() << std::endl;
    });

    std::jthread thd2([] {
        for (size_t i = 0; i < 10; ++i)
            std::cout << ThreadSafe::rand_double() << std::endl;
    });
}
// #endregion

int main()
{
    // use_unsafe_random() has a data race - run it only to demonstrate the problem
    use_thread_safe_random();
}
