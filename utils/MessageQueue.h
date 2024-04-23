#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class MessageQueue
{
public:
    using lock_type = std::unique_lock<std::mutex>;

public:
    MessageQueue() = default;

    ~MessageQueue() = default;

    template<typename IT>
    void push(IT &&item) {
        static_assert(std::is_same_v<T, std::decay_t<IT>>, "Item type is not convertible!!!");
        {
            lock_type lock{mutex_};
            queue_.emplace(std::forward<IT>(item));
        }
        cv_.notify_one();
    }

    auto pop() -> T {
        lock_type lock{mutex_};
        cv_.wait(lock, [&]() { return !queue_.empty(); });
        auto front = std::move(queue_.front());
        queue_.pop();
        return front;
    }

private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
};