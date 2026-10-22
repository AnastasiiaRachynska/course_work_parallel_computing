//
// Created by User on 15.10.2026.
//

#include "ThreadPool.h"


ThreadPool::ThreadPool(int numThreads) {
    is_working = true;
    for (int i = 0; i < numThreads; i++) threads.push_back(thread(worker, this));
}

void ThreadPool::worker(ThreadPool *pool) {
    while (true) {
        function<void()> task;
        {
            unique_lock<mutex> lock(pool->mtx);
            while (pool->is_working && pool->work_queue.empty()) {
                pool->cv.wait(lock);
            }
            if (!pool->is_working) return;
            task = pool->work_queue.front();
            pool->work_queue.pop();
        }

        task();
    }
}

void ThreadPool::terminate() {
    is_working = false;
    cv.notify_all();
    for (int i = 0; i < threads.size(); i++) threads[i].join();
}

void ThreadPool::addTask(function<void()> task) {
    bool added = false;
    {
        unique_lock<mutex> lock(mtx);
        if (work_queue.size() < 200) {
            work_queue.push(task);
            added = true;
        }
    }
    if (added) cv.notify_one();
}

ThreadPool::~ThreadPool() {
    terminate();
}
