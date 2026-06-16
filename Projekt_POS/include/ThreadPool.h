#pragma once

#include <functional>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

/**
 * @file ThreadPool.h
 * @brief Prosta pula wątków oparta o std::thread i kolejkę zadań.
 *
 * Implementacja korzysta wyłącznie ze standardowej biblioteki C++11,
 * bez zależności od WinAPI (przez co kompiluje się również na Linux/macOS).
 * Dla wymagania "Windows Threads" prowadzącego wystarczy podmienić
 * std::thread na HANDLE + CreateThread — interfejs klasy pozostaje bez zmian.
 */

/**
 * @class ThreadPool
 * @brief Pula wątków producent–konsument.
 *
 * Zadania (std::function<void()>) są kolejkowane i wykonywane przez
 * stałą liczbę wątków roboczych. Destruktor czeka na opróżnienie kolejki.
 *
 * Przykład użycia:
 * @code
 * ThreadPool pool(4);
 * for (auto& path : paths)
 *     pool.submit([&]{ processor.processImage(path, dst); });
 * pool.waitAll();
 * @endcode
 */
class ThreadPool
{
public:
    /**
     * @brief Tworzy pulę z podaną liczbą wątków i uruchamia je.
     * @param numThreads Liczba wątków roboczych (≥ 1).
     */
    explicit ThreadPool(int numThreads);

    /**
     * @brief Destruktor — czeka na zakończenie wszystkich wątków.
     */
    ~ThreadPool();

    /**
     * @brief Dodaje zadanie do kolejki.
     * @param task Callable bez argumentów i wartości zwracanej.
     */
    void submit(std::function<void()> task);

    /**
     * @brief Blokuje wątek wywołujący do czasu opróżnienia kolejki
     *        i zakończenia wszystkich aktywnych zadań.
     */
    void waitAll();

    // Brak kopiowania — pula jest właścicielem wątków
    ThreadPool(const ThreadPool&)            = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

private:
    /** @brief Funkcja wykonywana przez każdy wątek roboczy. */
    void workerLoop();

    std::vector<std::thread>          m_workers;    ///< Wątki robocze.
    std::queue<std::function<void()>> m_queue;      ///< Kolejka zadań.
    std::mutex                        m_mutex;      ///< Mutex kolejki.
    std::condition_variable           m_cv;         ///< Sygnał nowego zadania.
    std::condition_variable           m_cvDone;     ///< Sygnał zakończenia zadania.
    std::atomic<int>                  m_active{0};  ///< Liczba aktywnych zadań.
    bool                              m_stop{false};///< Flaga zatrzymania.
};
