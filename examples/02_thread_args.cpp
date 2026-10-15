#include "common.hpp"

#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

// #region by_value
void do_work(int value, std::string text)
{
    std::cout << "do_work(" << value << ", " << text << ")" << std::endl;
}

void pass_by_value()
{
    std::string text_data = "some data";
    std::thread thd{&do_work, 42, text_data};
    thd.join();
}
// #endregion

// #region by_reference
void produce(ThreadSafeQueue<Data>& q, const std::string& value)
{
    Data data = process_data(value);
    q.push(data);
}

void pass_by_reference()
{
    ThreadSafeQueue<Data> data_queue;
    const std::string value = "some data";

    std::thread thd{&produce, std::ref(data_queue), std::cref(value)};

    // ...
    thd.join();
}
// #endregion

// #region by_closure
void process_value(ThreadSafeQueue<Data>& q, std::string value)
{
    Data data = process_data(value);
    q.push(data);
}

void pass_by_closure()
{
    ThreadSafeQueue<Data> data_queue;
    const std::string value = "some data";

    std::thread thd{[&data_queue, value] { process_value(data_queue, value); }};

    thd.join();
}
// #endregion

// #region member_function
class Worker
{
    const int id_;

public:
    Worker(int id)
        : id_{id}
    {
    }

    void run(int n) const
    {
        std::cout << "Worker#" << id_ << " runs " << n << " times" << std::endl;
    }
};

void pass_member_function()
{
    Worker worker{1};

    std::thread thd{&Worker::run, &worker, 3};
    thd.join();
}
// #endregion

// #region move_semantics
void task(int id) { /* implementation */ }

std::thread create_thread() {
    static int id = 0;
    std::thread thd(&task, ++id);
    return thd;
}

void group_of_threads() {
    std::vector<std::thread> threads;

    // implicit move of r-value
    threads.push_back(create_thread());

    // explicit move of l-value
    std::thread thd(&task, 0);
    threads.push_back(std::move(thd));

    for (auto& thd : threads)
        thd.join();
}
// #endregion

int main()
{
    pass_by_value();
    pass_by_reference();
    pass_by_closure();
    pass_member_function();
    group_of_threads();
}
