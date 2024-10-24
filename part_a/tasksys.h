#ifndef _TASKSYS_H
#define _TASKSYS_H
#include <thread>
#include <vector>
#include <mutex>
#define MAX_THREADS 64

#include "itasksys.h"
#include <condition_variable>
#include <mutex>
#include <thread>
#include <queue>
#include <vector>
#include <atomic>

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
typedef struct
{
    IRunnable *runnable;
    int task_start_index;
    int task_end_index;
    int num_total_tasks;
    std::mutex *task_mutex;
    int *task_index;
} ParallelSpawnWorkerArgs;

class TaskSystemParallelSpawn : public ITaskSystem
{
public:
    TaskSystemParallelSpawn(int num_threads);
    ~TaskSystemParallelSpawn();
    int parallel_threads;
    std::vector<std::thread> workers;
    ParallelSpawnWorkerArgs workerArgs[MAX_THREADS];
    const char *name();
    void run(IRunnable *runnable, int num_total_tasks);
    TaskID runAsyncWithDeps(IRunnable *runnable, int num_total_tasks,
                            const std::vector<TaskID> &deps);
    void sync();
};

typedef struct
{
    IRunnable* runnable;
    int task_index;
    int num_total_tasks;
} Task;

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
        
    private:
        void parallelSpawnWorkerThreadSpinning(int id);
        std::vector<std::thread> pool_;
        std::queue<Task> unassigned_tasks_;
        std::atomic<bool> finished_;
        int num_threads_;
        int num_completed_;
        std::mutex task_q_mutex_; 
        std::mutex num_completed_mutex_;
};

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
    void parallelSpawnWorkerThreadSleeping(int id);
    std::vector<std::thread> pool_;
    std::queue<Task> unassigned_tasks_;
    std::atomic<bool> finished_;
    int num_threads_;
    int num_completed_;
    std::mutex task_q_mutex_; 
    std::mutex num_completed_mutex_;

    // std::mutex thread_wait_mutex_;
    // std::mutex run_wait_mutex_;
    std::condition_variable task_q_cv_;
    std::condition_variable num_completed_cv_;
};

#endif
