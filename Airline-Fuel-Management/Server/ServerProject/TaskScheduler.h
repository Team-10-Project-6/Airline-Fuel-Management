#pragma once

#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include <functional>

class TaskScheduler {
public:
    TaskScheduler(size_t threadNum);
    ~TaskScheduler();

	// pushes task to the telemetry processor
    void enqueueTask(std::function<void()> task);

    void stop();

private:
    boost::asio::thread_pool pool;
};