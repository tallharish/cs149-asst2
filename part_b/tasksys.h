#ifndef _TASKSYS_H
#define _TASKSYS_H

#include "itasksys.h"
#include <atomic>
#include <condition_variable>
#include <map>
#include <thread>
#include <queue>
#include <vector>
#include <unordered_set>

/*
 * TaskSystemSerial: This class is the student's implementation of a
 * serial task execution engine.  See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemSerial: public ITaskSystem {
    public:
        TaskSystemSerial(int num_threads);
        ~TaskSystemSerial();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

/*
 * TaskSystemParallelSpawn: This class is the student's implementation of a
 * parallel task execution engine that spawns threads in every run()
 * call.  See definition of ITaskSystem in itasksys.h for documentation
 * of the ITaskSystem interface.
 */
class TaskSystemParallelSpawn: public ITaskSystem {
    public:
        TaskSystemParallelSpawn(int num_threads);
        ~TaskSystemParallelSpawn();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

/*
 * TaskSystemParallelThreadPoolSpinning: This class is the student's
 * implementation of a parallel task execution engine that uses a
 * thread pool. See definition of ITaskSystem in itasksys.h for
 * documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSpinning: public ITaskSystem {
    public:
        TaskSystemParallelThreadPoolSpinning(int num_threads);
        ~TaskSystemParallelThreadPoolSpinning();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
};

typedef struct
{
    IRunnable* runnable;
    int task_index;
    int num_total_tasks;
} Task;

typedef struct
{
    IRunnable* runnable;
    int num_total_tasks;
} Task_group;

/*
 * TaskSystemParallelThreadPoolSleeping: This class is the student's
 * optimized implementation of a parallel task execution engine that uses
 * a thread pool. See definition of ITaskSystem in
 * itasksys.h for documentation of the ITaskSystem interface.
 */
class TaskSystemParallelThreadPoolSleeping: public ITaskSystem {
    public:
        TaskSystemParallelThreadPoolSleeping(int num_threads);
        ~TaskSystemParallelThreadPoolSleeping();
        const char* name();
        void run(IRunnable* runnable, int num_total_tasks);
        TaskID runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                const std::vector<TaskID>& deps);
        void sync();
    private:
        std::map<TaskID, std::unordered_set<TaskID>> parent_;
        std::map<TaskID, int> num_incomplete_parents_;
        std::map<TaskID, bool> task_completed_;
        std::map<TaskID, Task_group> task_lookup_; // adding to same lock!
        TaskID next_task_id_;
        std::unordered_set<TaskID> pending_tasks_;
        std::mutex task_management_mutex_; // handles read/writes to task_completed_ AND num_incomplete_parents;

        void parallelSpawnWorkerThreadSleeping(int id);
        std::vector<std::thread> pool_;
        std::queue<Task> unassigned_tasks_;
        std::atomic<bool> finished_;
        int num_threads_;
        int num_completed_;
        std::mutex task_q_mutex_; 
        std::mutex num_completed_mutex_;

        std::condition_variable task_q_cv_;
        std::condition_variable num_completed_cv_;
        void reset_num_completed();
        void add_tasks_queue(IRunnable* runnable, int num_total_tasks);
        void wait_unassigned_tasks(int num_total_tasks);
};

#endif
