#ifndef COURSE_WORK_PARALLEL_COMPUTING_THREADPOOL_H
#define COURSE_WORK_PARALLEL_COMPUTING_THREADPOOL_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

using namespace std;


class ThreadPool {
private:
    vector<thread> threads;
    queue<function<void()>> work_queue;
    mutex mtx;
    int active_tasks = 0;
    condition_variable cv_tasks_done;
    condition_variable cv_task_space;
    condition_variable cv;
    atomic<bool> is_working;
    static void worker(ThreadPool* pool);
public:
    ThreadPool(int numThreads);
    void terminate();
    void addTask(function<void()> task);
    void wait_all();
    ~ThreadPool() ;
};

#endif //COURSE_WORK_PARALLEL_COMPUTING_THREADPOOL_H