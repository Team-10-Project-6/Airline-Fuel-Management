#include "TaskScheduler.h"
#include <iostream>

using namespace std;

TaskScheduler::TaskScheduler(size_t threadNum) : pool(threadNum) {
    cout << ("Task Scheduler started with " + to_string(threadNum) + " worker threads.\n");
}

TaskScheduler::~TaskScheduler() {
    stop();
}

void TaskScheduler::enqueueTask(std::function<void()> task) {
    boost::asio::post(pool, task);
}

void TaskScheduler::stop() {
    pool.join();
}