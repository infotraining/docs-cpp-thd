#include "common.hpp"

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>

// Each variant lives in its own namespace, so that all of them can be compiled together.

namespace busy_wait_race
{
    // #region race_decl
    int data{0};
    bool is_data_ready{false};
    // #endregion

    // #region race_thd1
    void thread_1()
    {
        data = 42;

        is_data_ready = true;
    }
    // #endregion

    // #region race_thd2
    void thread_2()
    {
        while (!is_data_ready)
        {
        }

        assert(data == 42);
    }
    // #endregion

    // #region race_transformed_thd2
    void thread_2_transformed_by_compiler()
    {
        bool tmp = is_data_ready;
        while (!tmp)
        {
        }

        assert(data == 42); // assert fails
    }
    // #endregion
}

namespace busy_wait_atomic
{
    // #region atomic_decl
    int data{0};
    std::atomic<bool> is_data_ready{false};
    // #endregion

    // #region atomic_thd1
    void thread_1()
    {
        data = 42;

        is_data_ready = true;
    }
    // #endregion

    // #region atomic_thd2
    void thread_2()
    {
        while (!is_data_ready)
        {
        }

        assert(data == 42);
    }
    // #endregion

    // #region store_load_thd1
    void thread_1_store()
    {
        data = 42;

        is_data_ready.store(true);
    }
    // #endregion

    // #region store_load_thd2
    void thread_2_load()
    {
        while (!is_data_ready.load())
        {
        }

        assert(data == 42);
    }
    // #endregion
}

// #region spin_lock
class SpinLockMutex
{
    std::atomic_flag flag_;

public:
    SpinLockMutex() = default;

    void lock()
    {
        while (flag_.test_and_set(std::memory_order_acquire))
        {
        }
    }

    void unlock()
    {
        flag_.clear(std::memory_order_release);
    }
};
// #endregion

// #region atomic_flag_usage
void use_atomic_flag()
{
    std::atomic_flag f; // clear state since C++20

    bool prev_state = f.test_and_set();
    assert(prev_state == false);

    prev_state = f.test_and_set();
    assert(prev_state == true);

    f.clear(std::memory_order_release);
}
// #endregion

// #region atomic_bool_basic
void atomic_bool_basic()
{
    std::atomic<bool> b{true};

    b = false; // == b.store(false, std::memory_order_seq_cst);
    bool current_state = b; // == b.load(std::memory_order_seq_cst);
}
// #endregion

// #region atomic_bool_orders
void atomic_bool_orders(std::atomic<bool>& b)
{
    bool x = b.load(std::memory_order_acquire);

    b.store(true); // == b.store(true, std::memory_order_seq_cst);

    x = b.exchange(false, std::memory_order_acq_rel);
}
// #endregion

// #region cas_strong
void cas_strong(std::atomic<bool>& b) // b is set somewhere else
{
    bool expected = false;

    if (b.compare_exchange_strong(expected, true))
        std::cout << "b was set by me\n";
    else
        std::cout << "b was set by someone else\n";
}
// #endregion

// #region cas_weak
bool try_increment_below_limit(std::atomic<int>& counter, int limit)
{
    int expected = counter.load();

    while (expected < limit)
    {
        if (counter.compare_exchange_weak(expected, expected + 1))
            return true;
    }

    return false;
}
// #endregion

class LockFreeStack
{
    struct Node
    {
        int value{};
        Node* next{};
    };

    std::unique_ptr<Node[]> nodes_;
    const std::size_t capacity_;
    std::atomic<std::size_t> allocated_{0};
    std::atomic<Node*> head_{nullptr};

    // #region lockfree_stack_reserve_node
    Node* reserve_node()
    {
        auto index = allocated_.load(std::memory_order_relaxed);
        while (index < capacity_)
        {
            if (allocated_.compare_exchange_weak(
                    index, index + 1, std::memory_order_relaxed))
                return &nodes_[index];
        }
        return nullptr;
    }
    // #endregion

public:
    explicit LockFreeStack(std::size_t capacity)
        : nodes_(std::make_unique<Node[]>(capacity)), capacity_(capacity)
    {
    }

    // #region lockfree_stack_push
    bool push(int value)
    {
        Node* node = reserve_node();
        if (node == nullptr)
            return false;
        node->value = value;
        
        Node* expected = head_.load(std::memory_order_relaxed);
        do
        {
            node->next = expected;
        } while (!head_.compare_exchange_weak(
            expected, node, std::memory_order_release, std::memory_order_relaxed));

        return true;
    }
    // #endregion

    // #region lockfree_stack_pop
    bool try_pop(int& value)
    {
        Node* expected = head_.load(std::memory_order_acquire);
        while (expected != nullptr)
        {
            Node* next = expected->next;
            if (head_.compare_exchange_weak(
                    expected, next, std::memory_order_acq_rel, std::memory_order_acquire))
            {
                value = expected->value;
                return true;
            }
        }
        return false;
    }
    // #endregion
};

int main()
{
    // busy_wait_race has a data race - it is not run here

    {
        std::jthread t2{&busy_wait_atomic::thread_2};
        std::jthread t1{&busy_wait_atomic::thread_1};
    }

    busy_wait_atomic::is_data_ready = false;
    busy_wait_atomic::data = 0;
    {
        std::jthread t2{&busy_wait_atomic::thread_2_load};
        std::jthread t1{&busy_wait_atomic::thread_1_store};
    }

    SpinLockMutex spin;
    int counter = 0;
    {
        auto inc = [&] {
            for (int i = 0; i < 100'000; ++i)
            {
                std::lock_guard lk{spin};
                ++counter;
            }
        };
        std::jthread a{inc}, b{inc};
    }
    assert(counter == 200'000);

    use_atomic_flag();
    atomic_bool_basic();

    std::atomic<bool> b{false};
    atomic_bool_orders(b);
    b = false;
    cas_strong(b);

    std::atomic<int> bounded_counter{0};
    assert(try_increment_below_limit(bounded_counter, 1));
    assert(!try_increment_below_limit(bounded_counter, 1));

    constexpr int stack_capacity = 2'000;
    LockFreeStack stack{stack_capacity};
    {
        std::jthread producer_1{[&] {
            for (int i = 1; i <= stack_capacity / 2; ++i)
            {
                if (!stack.push(i))
                    std::abort();
            }
        }};
        std::jthread producer_2{[&] {
            for (int i = stack_capacity / 2 + 1; i <= stack_capacity; ++i)
            {
                if (!stack.push(i))
                    std::abort();
            }
        }};
    }

    std::atomic<int> popped_count{0};
    std::atomic<int> popped_sum{0};
    auto consume = [&] {
        int value;
        while (stack.try_pop(value))
        {
            popped_count.fetch_add(1);
            popped_sum.fetch_add(value);
        }
    };
    {
        std::jthread consumer_1{consume};
        std::jthread consumer_2{consume};
    }
    assert(popped_count == stack_capacity);
    assert(popped_sum == stack_capacity * (stack_capacity + 1) / 2);
}
