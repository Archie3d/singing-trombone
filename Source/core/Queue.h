#pragma once

#include <atomic>

namespace core {

/**
 * @brief Simple lock-free ring buffer.
 *
 * This is an implementation of single-producer single-consumer queue.
 *
 * @note From http://www.vitorian.com/x1/archives/370
 * https://github.com/Vitorian/RedditHelp/blob/master/test_spsc_ring.cpp
 */
template <typename T, size_t Size>
class Queue final
{
public:

    Queue() noexcept
        : readIdx {0},
          writeIdx {0},
          data {{}}
    {
    }

    Queue(const Queue&) = delete;
    Queue& operator =(const Queue&) = delete;

    ~Queue() = default;

    bool send(const T& obj) noexcept
    {
        size_t nextIdx = writeIdx + 1 < Size ? writeIdx + 1 : 0;

        if (nextIdx != readIdx) {
            data[writeIdx] = obj;
            writeIdx = nextIdx;
            return true;
        }

        return false;
    }

    bool receive(T& obj) noexcept
    {
        size_t nextIdx = readIdx + 1 < Size ? readIdx + 1 : 0;

        if (readIdx != writeIdx) {
            obj = std::move (data[readIdx]);
            readIdx = nextIdx;
            return true;
        }

        return false;
    }

    size_t count() const noexcept
    {
        return (writeIdx - readIdx) % Size;
    }

private:

    std::atomic<size_t> readIdx;
    std::atomic<size_t> writeIdx;
    T data[Size];
};

} // namespace core
