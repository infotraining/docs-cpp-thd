#include "common.hpp"

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

void prepare_data() { }
void process_data() { }

// Each variant lives in its own namespace, so that all of them can be compiled together.

namespace busy_wait_mutex
{
    // #region busy_mutex_decl
    bool data_ready = false;
    std::mutex mtx;
    // #endregion

    // #region busy_mutex_set
    void set_data_ready()
    {
        prepare_data();
        std::lock_guard<std::mutex> lk(mtx);
        data_ready = true;
    }
    // #endregion

    // #region busy_mutex_task
    void task()
    {
        bool ready_flag = false;

        do
        {
            std::lock_guard<std::mutex> lk{mtx};
            ready_flag = data_ready;
        } while (!ready_flag);

        process_data();
    }
    // #endregion
}

namespace busy_wait_atomic
{
    // #region busy_atomic_decl
    std::atomic<bool> data_ready{false};
    // #endregion

    // #region busy_atomic_set
    void set_data_ready()
    {
        prepare_data();
        data_ready = true;
    }
    // #endregion

    // #region busy_atomic_task
    void task()
    {
        while (!data_ready)
        {
            // ...
            std::this_thread::yield();
        }

        process_data();
    }
    // #endregion
}

namespace busy_wait_memory_order
{
    std::atomic<bool> data_ready{false};

    // #region busy_mo_set
    void set_data_ready()
    {
        prepare_data();
        data_ready.store(true, std::memory_order_release);
    }
    // #endregion

    // #region busy_mo_task
    void task()
    {
        while (!data_ready.load(std::memory_order_acquire))
        {
            // ...
            std::this_thread::yield();
        }

        process_data();
    }
    // #endregion
}

namespace idle_wait_cv
{
    // #region cv_decl
    bool data_ready = false;
    std::mutex mtx_ready;
    std::condition_variable cv_ready;
    // #endregion

    // #region cv_set
    void set_data_ready()
    {
        prepare_data();

        {
            std::lock_guard<std::mutex> lk(mtx_ready);
            data_ready = true;
        } // release lock

        cv_ready.notify_one();
    }
    // #endregion

    // #region cv_task
    void task()
    {
        std::unique_lock<std::mutex> lk(mtx_ready);
        while (!data_ready)
        {
            cv_ready.wait(lk);
        }
        lk.unlock();

        process_data();
    }
    // #endregion
}

namespace idle_wait_cv_predicate
{
    bool data_ready = false;
    std::mutex mtx_ready;
    std::condition_variable cv_ready;

    void set_data_ready()
    {
        prepare_data();

        {
            std::lock_guard<std::mutex> lk(mtx_ready);
            data_ready = true;
        } // release lock

        cv_ready.notify_one();
    }

    // #region cv_pred_task
    void task()
    {
        std::unique_lock<std::mutex> lk(mtx_ready);
        cv_ready.wait(lk, [] { return data_ready; });

        lk.unlock();

        process_data();
    }
    // #endregion
}

namespace idle_wait_atomic
{
    // #region atomic_wait_decl
    std::atomic<bool> data_ready = false;
    // #endregion

    // #region atomic_wait_set
    void set_data_ready()
    {
        prepare_data();

        data_ready = true;
        data_ready.notify_one();
    }
    // #endregion

    // #region atomic_wait_task
    void task()
    {
        data_ready.wait(false);

        process_data();
    }
    // #endregion
}

template <typename Namespace_Set, typename Namespace_Task>
void run(Namespace_Set set, Namespace_Task task)
{
    std::jthread consumer{task};
    std::this_thread::sleep_for(50ms);
    std::jthread producer{set};
}

int main()
{
    run(busy_wait_mutex::set_data_ready, busy_wait_mutex::task);
    run(busy_wait_atomic::set_data_ready, busy_wait_atomic::task);
    run(busy_wait_memory_order::set_data_ready, busy_wait_memory_order::task);
    run(idle_wait_cv::set_data_ready, idle_wait_cv::task);
    run(idle_wait_cv_predicate::set_data_ready, idle_wait_cv_predicate::task);
    run(idle_wait_atomic::set_data_ready, idle_wait_atomic::task);

    std::cout << "All events delivered" << std::endl;
}
