#ifndef _TASKSYS_H
#define _TASKSYS_H

#include "itasksys.h"
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <map>
#include <unordered_set>

/*
 * TaskSystemSerial: This class is the student's implementation of a
 * serial task execution engine.  See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemSerial : public ITaskSystem
{
public:
    TaskSystemSerial(int num_threads);
    ~TaskSystemSerial();
    const char *name();
    void run(IRunnable *runnable, int num_total_tasks);
    TaskID runAsyncWithDeps(IRunnable *runnable, int num_total_tasks,
                            const std::vector<TaskID> &deps);
    void sync();
};

/*
 * TaskSystemParallelSpawn: This class is the student's implementation of a
 * parallel task execution engine that spawns threads in every run()
 * call.  See definition of ITaskSystem in itasksys.h for documentation
 * of the ITaskSystem interface.
 */
class TaskSystemParallelSpawn : public ITaskSystem
{
public:
    TaskSystemParallelSpawn(int num_threads);
    ~TaskSystemParallelSpawn();
    const char *name();
    void run(IRunnable *runnable, int num_total_tasks);
    TaskID runAsyncWithDeps(IRunnable *runnable, int num_total_tasks,
                            const std::vector<TaskID> &deps);
    void sync();
};

/*
 * TaskSystemParallelThreadPoolSpinning: This class is the student's
 * implementation of a parallel task execution engine that uses a
 * thread pool. See definition of ITaskSystem in itasksys.h for
 * documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSpinning : public ITaskSystem
{
public:
    TaskSystemParallelThreadPoolSpinning(int num_threads);
    ~TaskSystemParallelThreadPoolSpinning();
    const char *name();
    void run(IRunnable *runnable, int num_total_tasks);
    TaskID runAsyncWithDeps(IRunnable *runnable, int num_total_tasks,
                            const std::vector<TaskID> &deps);
    void sync();
};

typedef struct
{
    IRunnable *runnable;
    int task_index;
    int num_total_tasks;
    TaskID BulkTask_id;
} Task;

typedef struct
{
    IRunnable *runnable;
    int num_total_tasks;
    int num_tasks_completed;
} BulkTask;

/*
 * TaskSystemParallelThreadPoolSleeping: This class is the student's
 * optimized implementation of a parallel task execution engine that uses
 * a thread pool. See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSleeping : public ITaskSystem
{
public:
    TaskSystemParallelThreadPoolSleeping(int num_threads);
    ~TaskSystemParallelThreadPoolSleeping();
    const char *name();
    void run(IRunnable *runnable, int num_total_tasks);
    TaskID runAsyncWithDeps(IRunnable *runnable, int num_total_tasks,
                            const std::vector<TaskID> &deps);
    void sync();

private:
    // Thread Pool
    void parallelSpawnWorkerThreadSleeping(int id);
    std::vector<std::thread> pool_;
    std::atomic<bool> finished_;
    int num_threads_;

    // Worker Tasks
    std::queue<Task> ready_q_;
    std::condition_variable ready_q_cv_;
    std::mutex ready_q_mutex_;

    // Tasks
    int num_BulkTask_completed_;
    int total_BulkTasks_;
    std::mutex task_completed_mutex_;
    std::condition_variable task_completed_cv_;

    // std::mutex BulkTask_scheduled_mutex_;
    // std::condition_variable BulkTask_scheduled_cv_;

    // BulkTasks
    // std::map<TaskID, bool> BulkTask_scheduled_;  // not needed
    std::map<TaskID, std::vector<TaskID>> BulkTask_children_; // Note Child, not parent
    std::map<TaskID, bool> BulkTask_completed_;
    std::map<TaskID, int> num_incomplete_parents_;
    std::map<TaskID, BulkTask> BulkTask_lookup_;
    TaskID next_BulkTask_id_;
    // std::mutex BulkTask_mutex_;

    std::mutex BulkTask_lookup_mutex_;
    std::mutex dep_mutex_;
    // std::condition_variable BulkTask_cv_;

    int unscheduled_BulkTasks;

    // New Methods
    void add_tasks_ready_q(std::vector<Task> tasks);
    void on_task_complete(TaskID BulkTask_id);
};

#endif
