#include "common.hpp"

#include <atomic>
#include <barrier>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <latch>
#include <string>
#include <syncstream>
#include <thread>
#include <vector>

// #region latch_task
void background_work(const int id, const std::string text,
                     std::latch& all_ready, std::latch& all_done)
{
    all_ready.arrive_and_wait();

    std::cout << "BW#" << id << " started..." << std::endl;

    for (const auto& letter : text)
    {
        std::cout << "Thread#" << id << " - " << letter << std::endl;
        std::this_thread::sleep_for(25ms);
    }

    all_done.count_down();
}
// #endregion

// #region latch_usage
void use_latch()
{
    std::latch all_ready(4);
    std::latch all_done(4);
    std::vector<std::jthread> thds;

    for (int i = 0; i < 4; ++i)
        thds.emplace_back([id = i + 1, &all_ready, &all_done] {
            background_work(id, "LATCH", all_ready, all_done);
        });

    all_done.wait();
}
// #endregion

// #region barrier
void use_barrier()
{
    const auto workers = {"A", "B", "C"};
    int counter = 0;

    auto on_completion = [&counter]() noexcept {
        // locking is not needed here
        ++counter;
        std::string phase = "--- done no. "s + std::to_string(counter) + "\n";
        std::cout << phase;
    };

    std::barrier sync_point(std::ssize(workers), on_completion);

    auto work = [&sync_point](std::string name) {
        for (int i = 0; i < 4; ++i)
        {
            std::string product = ".." + name + " worked\n";
            std::cout << product;
            sync_point.arrive_and_wait();
        }
    };

    std::vector<std::jthread> thds;
    for (const auto& worker : workers)
        thds.emplace_back(work, worker);
}
// #endregion

// #region atomic_ref
void use_atomic_ref()
{
    const int thds_count = 10;
    std::latch all_ready{thds_count};
    std::vector<uint32_t> data(10);

    {
        std::vector<std::jthread> thds;

        for (int t = 0; t < thds_count; ++t)
            thds.emplace_back([&] {
                all_ready.arrive_and_wait();

                for (int i = 0; i < 100'000; ++i)
                {
                    for (auto& item : data)
                    {
                        // ++item; // BUG!!! -> data-race
                        std::atomic_ref synced_value{item}; // data-race free access
                        ++synced_value;
                    }
                }
            });
    }

    assert(data[0] == thds_count * 100'000);
}
// #endregion

// #region osyncstream
void do_calculations(int n)
{
    std::osyncstream synced_cout{std::cout};

    for (int i = 0; i < n; ++i)
    {
        synced_cout
            << "Calculated value is "
            << fibonacci(i) << "\n"
            << std::flush_emit;
    }
}

void use_osyncstream()
{
    std::jthread thd_1{do_calculations, 10};
    std::jthread thd_2{do_calculations, 10};
}
// #endregion

int main()
{
    use_latch();
    use_barrier();
    use_atomic_ref();
    use_osyncstream();
}
