#include "tasksys.h"
#include <iostream>


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
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

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

TaskSystemParallelSpawn::TaskSystemParallelSpawn(int num_threads): ITaskSystem(num_threads) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelSpawn::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
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

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads): ITaskSystem(num_threads) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
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
    _threads = new std::thread[num_threads];
    for (int i = 0; i < _num_threads; i++) {
        _threads[i] = std::thread(
            &TaskSystemParallelThreadPoolSleeping::thread_task, this);
    }
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    std::unique_lock<std::mutex> lck(_mutex);
    _endThreads = true;
    _worker_cv.notify_all();
    lck.unlock();
    for (int i = 0; i < _num_threads; i++) {
        _threads[i].join();
    }
    delete[] _threads;
}

void TaskSystemParallelThreadPoolSleeping::thread_task() {
    std::unique_lock<std::mutex> lck(_mutex);
    int index;
    std::list<TaskSystemParallelThreadPoolSleeping::BulkMeta>::iterator cur;
    cur = _runnable_bulks.begin();
    while (!_endThreads) {
        /* Find a runnable bulk, if we've iterated the whole list, go sleep. */
        while (cur == _runnable_bulks.end()) {
            _worker_cv.wait(lck);
            if (_endThreads) return;
            /* After woken up, re-iterate the list from the beginning. */
            cur = _runnable_bulks.begin();
        }
        index = cur->_doing_cnt;

        /* Invalid _doing_cnt means the bulk is being/has been finished, so
         * iterate the _runnable_bulks list and try to find the next one. */
        while (index >= cur->_ntask) {
            cur++;
            while (cur == _runnable_bulks.end()) {
                _worker_cv.wait(lck);
                if (_endThreads) return;
                cur = _runnable_bulks.begin();
            }
            index = cur->_doing_cnt;
        }
        /* We've got a valid task index in this bulk. Run it! */
        cur->_doing_cnt++;
        lck.unlock();

        cur->_runnable->runTask(index, cur->_ntask);

        lck.lock();
        /* We've done the last task in this bulk. Mark the bulk as finished,
         * then try to find if another bulk is runnable. */
        if (++cur->_done_cnt == cur->_ntask) {
            _finished_bulks.insert(cur->_bulk_id);
            cur = _runnable_bulks.erase(cur);
            /* wake up main thread */
            _main_cv.notify_one();

            _findRunnableBulk();
        }
    }
}

void TaskSystemParallelThreadPoolSleeping::_findRunnableBulk() {
    bool isRunnable = true;
    for (auto it = _pending_bulks.begin(); it != _pending_bulks.end();) {
        for (auto &e : it->_deps) {
            if (_finished_bulks.find(e) == _finished_bulks.end()) {
                isRunnable = false;
            }
        }
        if (isRunnable) {
            _runnable_bulks.push_back(*it);
            it = _pending_bulks.erase(it);
        } else {
            ++it;
        }
    }
    _worker_cv.notify_all();
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {
    runAsyncWithDeps(runnable, num_total_tasks, {});
    sync();
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                    const std::vector<TaskID>& deps) {
    /* We must take the lock for list insertion and `_findRunnableBulk()`. */
    std::unique_lock<std::mutex> lck(_mutex);
    int bulk_id = _bulk_id++;
    _pending_bulks.push_back({runnable, num_total_tasks, deps, 0, 0, bulk_id});
    _findRunnableBulk();
    return bulk_id;
}

void TaskSystemParallelThreadPoolSleeping::sync() {
    std::unique_lock<std::mutex> lck(_mutex);
    /* Wait for previous bulks to finish. */
    while (!_runnable_bulks.empty() || !_pending_bulks.empty()) {
        _main_cv.wait(lck);
    }
}
