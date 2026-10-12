#include "common.hpp"

#include <algorithm>
#include <iostream>
#include <thread>
#include <vector>

// #region hardware_concurrency
void create_threads_vector()
{
    auto hardware_threads_count =
        std::max(1u, std::thread::hardware_concurrency());

    std::vector<std::thread> threads(hardware_threads_count);
}
// #endregion

int main()
{
    create_threads_vector();
}
