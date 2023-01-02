#include "tasksys.h"
#include "itasksys.h"
#include <iostream>
#include <mutex>
#include <thread>

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
 * Parallel Thread Pool Spinning Task System Implementation (Mutex)
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSpinningMutex::name() {
    return "Parallel + Thread Pool + Spin + Mutex";
}

TaskSystemParallelThreadPoolSpinningMutex::
    TaskSystemParallelThreadPoolSpinningMutex(int num_threads)
    : ITaskSystem(num_threads), _num_threads(num_threads) {
    _threads = new std::thread[_num_threads];
    for (int i = 0; i < _num_threads; i++) {
        _threads[i] = std::thread(
            &TaskSystemParallelThreadPoolSpinningMutex::thread_task, this);
    }
}

TaskSystemParallelThreadPoolSpinningMutex::
    ~TaskSystemParallelThreadPoolSpinningMutex() {
    _endThreads = true;
    for (int i = 0; i < _num_threads; i++) {
        _threads[i].join();
    }
    delete[] _threads;
}

void TaskSystemParallelThreadPoolSpinningMutex::thread_task() {
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

void TaskSystemParallelThreadPoolSpinningMutex::run(IRunnable *runnable,
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

TaskID TaskSystemParallelThreadPoolSpinningMutex::runAsyncWithDeps(
    IRunnable *runnable, int num_total_tasks, const std::vector<TaskID> &deps) {
    return 0;
}

void TaskSystemParallelThreadPoolSpinningMutex::sync() {
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Spinning Task System Implementation (Atomic)
 * ================================================================
 */

const char* TaskSystemParallelThreadPoolSpinningAtomic::name() {
    return "Parallel + Thread Pool + Spin + Atomic";
}

TaskSystemParallelThreadPoolSpinningAtomic::
    TaskSystemParallelThreadPoolSpinningAtomic(int num_threads)
    : ITaskSystem(num_threads), _num_threads(num_threads) {
    _threads = new std::thread[_num_threads];
    for (int i = 0; i < _num_threads; i++) {
        _threads[i] = std::thread(
            &TaskSystemParallelThreadPoolSpinningAtomic::thread_task, this);
    }
}

TaskSystemParallelThreadPoolSpinningAtomic::
    ~TaskSystemParallelThreadPoolSpinningAtomic() {
    _endThreads = true;
    for (int i = 0; i < _num_threads; i++) {
        _threads[i].join();
    }
    delete[] _threads;
}

void TaskSystemParallelThreadPoolSpinningAtomic::thread_task() {
    int index;
    while (!_endThreads) {
        if (!_haveTasks) {
            continue;
        }

        index = _doing_cnt.fetch_add(1);
        /* Buggy! What if _ntask is modified in run()? */
        if (index >= _ntask) {
            continue;
        }
        _runnable->runTask(index, _ntask);

        _done_cnt.fetch_add(1);
    }
}

void TaskSystemParallelThreadPoolSpinningAtomic::run(IRunnable *runnable,
                                                     int num_total_tasks) {
    _ntask = num_total_tasks;
    _runnable = runnable;

    _doing_cnt = _done_cnt = 0;

    _haveTasks = true;
    while (_done_cnt != _ntask)
        ;
    _haveTasks = false;
}

TaskID TaskSystemParallelThreadPoolSpinningAtomic::runAsyncWithDeps(
    IRunnable *runnable, int num_total_tasks, const std::vector<TaskID> &deps) {
    return 0;
}

void TaskSystemParallelThreadPoolSpinningAtomic::sync() {
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
    _endThreads = true;
    _cv.notify_all();
    for (int i = 0; i < _num_threads; i++) {
        _threads[i].join();
    }
    delete[] _threads;
}

void TaskSystemParallelThreadPoolSleeping::thread_task() {
    while (true) {
        std::unique_lock<std::mutex> lck(_mutex);

        int index = _doing_cnt;
        while (index >= _ntask) {
            _cv.wait(lck);
            if (_endThreads) return;
            index = _doing_cnt;
        }
        _doing_cnt++;

        lck.unlock();

        _runnable->runTask(index, _ntask);

        lck.lock();
        if (++_done_cnt == _ntask) {
            /* wake up main thread */
            _cv.notify_all();
        }
        lck.unlock();
    }
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {
    std::unique_lock<std::mutex> lck(_mutex);
    _ntask = num_total_tasks;
    _runnable = runnable;
    _doing_cnt = _done_cnt = 0;

    /* wake up workers */
    _cv.notify_all();
    while (_done_cnt != _ntask) {
        _cv.wait(lck);
    }
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
