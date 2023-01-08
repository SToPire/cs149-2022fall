#include "tasksys.h"
#include "itasksys.h"
#include <iostream>
#include <mutex>
#include <thread>
#include <cassert>

IRunnable::~IRunnable() {}

ITaskSystem::ITaskSystem(int num_threads) {}
ITaskSystem::~ITaskSystem() {}

/*
 * ================================================================
 * Serial task system implementation
 * ================================================================
 */

const char* TaskSystemSerial::name() {
    return "Serial";
}

TaskSystemSerial::TaskSystemSerial(int num_threads): ITaskSystem(num_threads) {
}

TaskSystemSerial::~TaskSystemSerial() {}

void TaskSystemSerial::run(IRunnable* runnable, int num_total_tasks) {
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemSerial::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                          const std::vector<TaskID>& deps) {
    return 0;
}

void TaskSystemSerial::sync() {
    return;
}

/*
 * ================================================================
 * Parallel Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelSpawn::name() {
    return "Parallel + Always Spawn";
}

TaskSystemParallelSpawn::TaskSystemParallelSpawn(int num_threads)
    : ITaskSystem(num_threads), _num_threads(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::thread_task(IRunnable *runnable,
                                          std::atomic<int> &counter,
                                          int num_total_tasks) {
    while (true) {
        int i = counter.fetch_add(1);
        if (i >= num_total_tasks) {
            break;
        }
        runnable->runTask(i, num_total_tasks);
    }
}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {
    std::atomic<int> counter(0);
    std::thread *threads = new std::thread[_num_threads];

    for (int i = 0; i < _num_threads; i++) {
        threads[i] = std::thread(&TaskSystemParallelSpawn::thread_task, this,
                                 runnable, std::ref(counter), num_total_tasks);
    }

    for (int i = 0; i < _num_threads; i++) {
        threads[i].join();
    }

    delete[] threads;
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    return 0;
}

void TaskSystemParallelSpawn::sync() {
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Spinning Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSpinning::name() {
    return "Parallel + Thread Pool + Spin";
}

TaskSystemParallelThreadPoolSpinning::
    TaskSystemParallelThreadPoolSpinning(int num_threads)
    : ITaskSystem(num_threads), _num_threads(num_threads) {
    _threads = new std::thread[_num_threads];
    for (int i = 0; i < _num_threads; i++) {
        _threads[i] = std::thread(
            &TaskSystemParallelThreadPoolSpinning::thread_task, this);
    }
}

TaskSystemParallelThreadPoolSpinning::
    ~TaskSystemParallelThreadPoolSpinning() {
    _endThreads = true;
    for (int i = 0; i < _num_threads; i++) {
        _threads[i].join();
    }
    delete[] _threads;
}

void TaskSystemParallelThreadPoolSpinning::thread_task() {
    int index;
    while (!_endThreads) {
        _mutex.lock();
        if (!_haveTasks) {
            _mutex.unlock();
            continue;
        }

        index = _doing_cnt;
        if (index >= _ntask) {
            _mutex.unlock();
            continue;
        }
        _doing_cnt++;
        _mutex.unlock();

        _runnable->runTask(index, _ntask);

        _mutex.lock();
        _done_cnt++;
        _mutex.unlock();
    }
}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable *runnable,
                                                    int num_total_tasks) {
    bool doing = true;

    _mutex.lock();

    _ntask = num_total_tasks;
    _runnable = runnable;

    _doing_cnt = _done_cnt = 0;

    _haveTasks = true;

    _mutex.unlock();

    /* spinning until all tasks are done */
    while (doing) {
        _mutex.lock();
        doing = _done_cnt != _ntask;
        _mutex.unlock();
    }

    _haveTasks = false;
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(
    IRunnable *runnable, int num_total_tasks, const std::vector<TaskID> &deps) {
    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Sleeping Task System Implementation
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSleeping::name() {
    return "Parallel + Thread Pool + Sleep";
}

TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(
    int num_threads)
    : ITaskSystem(num_threads), _num_threads(num_threads) {
    _threads = new std::thread[_num_threads];
    for (int i = 0; i < _num_threads; i++) {
        _threads[i] = std::thread(
            &TaskSystemParallelThreadPoolSleeping::thread_task, this);
    }
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    _mutex.lock();
    _endThreads = true;
    /* We must hold the lock to protect the critical section when signaling all
     * workers, otherwise, worker may receive the signal before it goes to
     * sleep, and it will sleep forever. */
    _worker_cv.notify_all();
    _mutex.unlock();
    for (int i = 0; i < _num_threads; i++) {
        _threads[i].join();
    }
    delete[] _threads;
}

void TaskSystemParallelThreadPoolSleeping::thread_task() {
    std::unique_lock<std::mutex> lck(_mutex);
    while (!_endThreads) {
        int index = _doing_cnt;
        while (index >= _ntask) {
            _worker_cv.wait(lck);
            if (_endThreads) return;
            index = _doing_cnt;
        }
        _doing_cnt++;

        lck.unlock();

        _runnable->runTask(index, _ntask);

        lck.lock();
        if (++_done_cnt == _ntask) {
            /* wake up main thread */
            _main_cv.notify_one();
        }
    }
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {
    std::unique_lock<std::mutex> lck(_mutex);
    _ntask = num_total_tasks;
    _runnable = runnable;
    _doing_cnt = _done_cnt = 0;

    /* wake up workers */
    _worker_cv.notify_all();
    /* put main thread into sleep*/
    _main_cv.wait(lck);
    /* assertion failure indicates a bug */
    assert(_done_cnt == _ntask);
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {


    //
    // TODO: CS149 students will implement this method in Part B.
    //

    return 0;
}

void TaskSystemParallelThreadPoolSleeping::sync() {

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    return;
}
