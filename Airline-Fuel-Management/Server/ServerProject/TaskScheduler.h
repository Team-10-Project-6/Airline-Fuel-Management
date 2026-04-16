/**
 * @file TaskScheduler.h
 * @brief Thread-pool wrapper that queues and executes server tasks concurrently.
 */
#pragma once

#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include <functional>

/**
 * @class TaskScheduler
 * @brief Provides an interface to enqueue asynchronous tasks onto a Boost ASIO thread pool.
 */
class TaskScheduler {
public:
    /**
     * @brief Constructs a new Task Scheduler.
     * @param threadNum The number of concurrent threads to spawn in the pool.
     */
    TaskScheduler(size_t threadNum);

    /**
     * @brief Destroys the Task Scheduler, waiting for pending tasks to finish.
     */
    ~TaskScheduler();

	/**
     * @brief Pushes a new task lambda onto the thread pool queue.
     * @param task A std::function wrapping the void() task logic.
     */
    void enqueueTask(std::function<void()> task);

    /**
     * @brief Force stops the thread pool, dropping any tasks not yet started.
     */
    void stop();

private:
    boost::asio::thread_pool pool;
};