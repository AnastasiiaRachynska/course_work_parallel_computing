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
            while (pool->is_working && pool->work_queue.empty())
                pool->cv_task_available.wait(lock);
            if (!pool->is_working && pool->work_queue.empty()) return;
            task = pool->work_queue.front();
            pool->work_queue.pop();
            pool->active_tasks++;
        }
        pool->cv_task_space.notify_one();
        task();

        {
            lock_guard<mutex> lock(pool->mtx);
            pool->active_tasks--;
        }
        pool->cv_tasks_done.notify_one();
    }
}

void ThreadPool::terminate() {
    {
        lock_guard<mutex> lock(mtx);
        is_working = false;
    }
    cv_task_available.notify_all();
    for (int i = 0; i < threads.size(); i++) threads[i].join();
}

void ThreadPool::add_task(function<void()> task) {
    {
        unique_lock<mutex> lock(mtx);
        while (work_queue.size() >= 200)
            cv_task_space.wait(lock);
        work_queue.push(task);
    }
    cv_task_available.notify_one();
}

void ThreadPool::wait_all() {
    unique_lock<mutex> lock(mtx);
    while (!work_queue.empty() || active_tasks > 0)
        cv_tasks_done.wait(lock);
}

ThreadPool::~ThreadPool() {
    terminate();
}
