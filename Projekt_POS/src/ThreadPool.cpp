/**
 * @file ThreadPool.cpp
 * @brief Implementacja puli wątków
 */

#include "ThreadPool.h"

// ---------------------------------------------------------------------------

ThreadPool::ThreadPool(int numThreads)
{
    for (int i = 0; i < numThreads; ++i)
        m_workers.emplace_back(&ThreadPool::workerLoop, this);
}

// ---------------------------------------------------------------------------

ThreadPool::~ThreadPool()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stop = true;
    }
    // Poinformuj wszystkie watki, zeby mogly sprawdzic flagę m_stop
    m_cv.notify_all();
    for (auto& t : m_workers)
        if (t.joinable()) t.join();
}

// ---------------------------------------------------------------------------

void ThreadPool::submit(std::function<void()> task)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(std::move(task));
    }
    m_cv.notify_one();
}

// ---------------------------------------------------------------------------

void ThreadPool::waitAll()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cvDone.wait(lock, [this]     // Czekaj dopoki kolejka nie jest pusta LUB jakieś zadanie jest aktywne

    {
        return m_queue.empty() && m_active.load() == 0;
    });
}

// ---------------------------------------------------------------------------

void ThreadPool::workerLoop()
{
    while (true)
    {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(m_mutex);

            m_cv.wait(lock, [this]      // Czekaj na nowe zadanie lub sygnal zatrzymania
            {
                return !m_queue.empty() || m_stop;
            });

            if (m_stop && m_queue.empty()) return;

            task = std::move(m_queue.front());
            m_queue.pop();
            ++m_active;
        }

        task();         // Wykonaj zadanie poza sekcja krytyczna


        {
            std::lock_guard<std::mutex> lock(m_mutex);
            --m_active;
        }
        // Powiadom waitAll() że zadanie zakończone
        m_cvDone.notify_all();
    }
}
