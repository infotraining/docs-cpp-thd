#include "common.hpp"

#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <optional>

// #region lock_guard
template <typename Value>
struct Synchronized
{
    Value value;
    std::mutex mtx_value;
};

template <typename F, typename Value>
void apply(F f, Synchronized<Value>& sync_value)
{
    std::lock_guard<std::mutex> lk{sync_value.mtx_value};
    f(sync_value.value);
}
// #endregion

// #region lock_guard_usage
void use_synchronized(Synchronized<std::vector<int>>& sync_vec)
{
    apply([](std::vector<int>& v) {
            v.push_back(1);
            v.push_back(2);
        }, sync_vec);
}
// #endregion

// #region timed_mutex
void try_lock_with_timeout(std::timed_mutex& mtx)
{
    std::unique_lock lk(mtx, std::try_to_lock);

    if (!lk.owns_lock())
    {
        do
        {
            std::cout << "Thread does not own a lock..."
                      << " Tries to acquire a mutex..."
                      << std::endl;
        } while (!lk.try_lock_for(1s));
    }

    std::cout << "Lock acquired" << std::endl;
}
// #endregion

// #region shared_mutex_usage
#include <shared_mutex>
#include <unordered_map>

class DnsCache {
public:
    void update(std::string host, std::string ip) {
        std::unique_lock lock(m_);        // writer
        cache_[std::move(host)] = std::move(ip);
    }

    std::optional<std::string> resolve(const std::string& host) const {
        std::shared_lock lock(m_);        // reader
        if (auto it = cache_.find(host); it != cache_.end())
            return it->second;
        return std::nullopt;
    }

private:
    mutable std::shared_mutex m_;
    std::unordered_map<std::string, std::string> cache_;
};
// #endregion

int main()
{
    Synchronized<std::vector<int>> sync_vec;

    use_synchronized(sync_vec);

    std::timed_mutex mtx;
    std::jthread holder{[&mtx] {
        std::lock_guard lk{mtx};
        std::this_thread::sleep_for(2500ms);
    }};
    std::this_thread::sleep_for(100ms);

    try_lock_with_timeout(mtx);
}
