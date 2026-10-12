#include "common.hpp"

#include <atomic>
#include <cassert>
#include <exception>
#include <functional>
#include <future>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

// #region async_callables
int fib(int n)
{
    return static_cast<int>(fibonacci(n));
}

void async_callables()
{
    Gadget g{42, "ipad"};

    std::future<int> f1 = std::async(std::launch::async, fib, 10);
    std::future<int> f2 = std::async(std::launch::async, [] { return fib(20); });
    std::future<int> f3 = std::async(std::launch::async, [g] { return g.use(); });
    std::future<int> f4 = std::async(std::launch::async, &Gadget::use, g);
}
// #endregion

// #region async_args
void work(int id, const std::string& path, std::atomic<int>& counter)
{
    std::cout << "work#" << id << " in " << path << " - " << ++counter << std::endl;
}

void async_args()
{
    int id = 1;
    const std::string path = "/dev/home/";
    std::atomic<int> counter{0};

    auto f1 = std::async(std::launch::async,
                         &work, id, std::cref(path), std::ref(counter));

    auto f2 = std::async(std::launch::async,
                         [id, &path, &counter] { work(id, path, counter); });
}
// #endregion

// #region async_destructor_bad
void save_to_file(const std::string& path, const std::string& msg)
{
    std::this_thread::sleep_for(500ms);
    std::cout << "Saved to " << path << ": " << msg << std::endl;
}

void async_destructor_bad()
{
    // each temporary future blocks in its destructor - the calls run one after another
    std::async(std::launch::async, &save_to_file, "a.log", "Do not do");
    std::async(std::launch::async, &save_to_file, "b.log", "like THIS");
}
// #endregion

// #region async_destructor_good
void async_destructor_good()
{
    auto f1 = std::async(std::launch::async, &save_to_file, "a.log", "Do LIKE");
    auto f2 = std::async(std::launch::async, &save_to_file, "b.log", "THIS");
}
// #endregion

// #region packaged_task
void use_packaged_task()
{
    std::packaged_task<int()> ptask1([] { return calculate(665); });

    std::future<int> f1 = ptask1.get_future();

    std::jthread thd1(std::move(ptask1));

    try
    {
        auto result = f1.get();
        std::cout << "Result: " << result << std::endl;
    }
    catch (const std::runtime_error& e)
    {
        // handle the exception
    }
}
// #endregion

// #region promise_data_source
class DataSource
{
    std::vector<std::promise<int>> channels_;

public:
    DataSource(size_t channels_count)
        : channels_(channels_count)
    {
    }

    std::future<int> get_future_channel(size_t channel_no)
    {
        return channels_.at(channel_no).get_future();
    }

    void read_data()
    {
        for (size_t i = 0; i < channels_.size(); ++i)
        {
            std::this_thread::sleep_for(25ms);
            try
            {
                auto result = calculate(i);
                channels_[i].set_value(result);
            }
            catch (...)
            {
                std::exception_ptr eptr = std::current_exception();
                channels_[i].set_exception(eptr);
            }
        }
    }
};
// #endregion

// #region promise_usage
void use_promise()
{
    const size_t channel_count = 16;
    DataSource data_source(channel_count);

    std::future<int> future_channel_4 = data_source.get_future_channel(4);
    std::future<int> future_channel_13 = data_source.get_future_channel(13);

    std::jthread thd{[ds = std::move(data_source)]() mutable { ds.read_data(); }};

    assert(future_channel_4.get() == 16);

    try
    {
        future_channel_13.get();
        assert(false); // channel 13 reports an exception
    }
    catch (const std::runtime_error& e)
    {
        std::cout << "Channel 13: " << e.what() << std::endl;
    }
}
// #endregion

int main()
{
    async_callables();
    async_args();
    async_destructor_bad();
    async_destructor_good();
    use_packaged_task();
    use_promise();
}
