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
    condition_variable cv;
    atomic<bool> is_working;
    static void worker(ThreadPool* pool);
public:
    ThreadPool(int numThreads);
    void terminate();
    void addTask(function<void()> task);
    ~ThreadPool() ;
};

#endif //COURSE_WORK_PARALLEL_COMPUTING_THREADPOOL_H