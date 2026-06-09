#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <type_traits>
#include <stdexcept>

class ThreadPool
{
public:
    ThreadPool(size_t threadCount)
        : m_Stop(false)
    {
        for (size_t i = 0; i < threadCount; i++)
        {
            m_Workers.emplace_back([this]()
                {
                    WorkerLoop();
                });
        }
    }

    ~ThreadPool()
    {
        Shutdown();
    }

    template <typename Func, typename... Args>
    auto Enqueue(Func&& func, Args&&... args)
        -> std::future<std::invoke_result_t<Func, Args...>>
    {
        using ReturnType = std::invoke_result_t<Func, Args...>;

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
        );

        std::future<ReturnType> result = task->get_future();

        {
            std::unique_lock<std::mutex> lock(m_QueueMutex);

            if (m_Stop)
            {
                throw std::runtime_error("Cannot enqueue task after shutdown");
            }

            m_Tasks.emplace([task]()
                {
                    (*task)();
                });
        }

        m_Condition.notify_one();

        return result;
    }

    void Shutdown()
    {
        {
            std::unique_lock<std::mutex> lock(m_QueueMutex);

            if (m_Stop)
            {
                return;
            }

            m_Stop = true;
        }

        m_Condition.notify_all();

        for (std::thread& worker : m_Workers)
        {
            if (worker.joinable())
            {
                worker.join();
            }
        }
    }

private:
    void WorkerLoop()
    {
        while (true)
        {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(m_QueueMutex);

                m_Condition.wait(lock, [this]()
                    {
                        return m_Stop || !m_Tasks.empty();
                    });

                if (m_Stop && m_Tasks.empty())
                {
                    return;
                }

                task = std::move(m_Tasks.front());
                m_Tasks.pop();
            }

            task();
        }
    }

private:
    std::vector<std::thread> m_Workers;
    std::queue<std::function<void()>> m_Tasks;

    std::mutex m_QueueMutex;
    std::condition_variable m_Condition;

    bool m_Stop;
};