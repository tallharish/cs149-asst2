#include "tasksys.h"
#define DEFAULT_NUM_THREADS 1

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
    // You do not need to implement this method.
    return 0;
}

void TaskSystemSerial::sync() {
    // You do not need to implement this method.
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
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //

    // Harish - Step B - create num_threads and keep them handy. Add new class member variables to keep track of the threads. 
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

typedef struct
{
    IRunnable* runnable;
    int task_start_index;
    int task_end_index;
    int num_total_tasks;
} WorkerArgs;

void parallelSpawnWorkerThread(WorkerArgs *const args) {
    for (int i = args->task_start_index; i < args->task_end_index; i++) {
        args->runnable->runTask(i, args->num_total_tasks);
    }
}

void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {

    
    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    // Harish - Step A - Create num_threads threads, assign runTask to each thread in a round-robin manner. Finally, release all the threads. Run test suite and measure performance. 
    std::thread workers[DEFAULT_NUM_THREADS];
    WorkerArgs args[DEFAULT_NUM_THREADS];

    // Harish - Step C - use the thread pool (from step 2) to create this. Not sure what variables are shared between threads? 

    //int tasks_per_thread = num_total_tasks/DEFAULT_NUM_THREADS;
    // for (int i = 0; i < num_total_tasks; i++) {
    //     runnable->runTask(i, num_total_tasks);
    // }

    int tasks_per_thread = (num_total_tasks + DEFAULT_NUM_THREADS - 1) / DEFAULT_NUM_THREADS;
    for (int i = 0; i < num_total_tasks; i += tasks_per_thread) {
        args[i / tasks_per_thread] = {runnable, i, std::min(num_total_tasks, i + tasks_per_thread), num_total_tasks};
        workers[i / tasks_per_thread] = std::thread(parallelSpawnWorkerThread, args + i / tasks_per_thread);
    }

    for (int j = 0; j < num_total_tasks; j += tasks_per_thread) {
        workers[j / tasks_per_thread].join();
    }
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                 const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemParallelSpawn::sync() {
    // You do not need to implement this method.
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

typedef struct
{
    int id;
    PoolAssignment* assignments;
    bool* finished;
    std::mutex* mutex;
    int* still_running;
} SpinningWorkerArgs;

void parallelSpawnWorkerThreadSpinning(SpinningWorkerArgs* args) {
    while (!(*(args->finished))) {
        if (args->assignments[args->id].assigned) {
            for (int i = args->assignments[args->id].task_start_index; i < args->assignments[args->id].task_end_index; i++) {
                args->assignments[args->id].runnable->runTask(i, args->assignments[args->id].num_total_tasks);
            }
            args->assignments[args->id].assigned = false;
            args->mutex->lock();
            *(args->still_running) -= 1;
            args->mutex->unlock();
        }
        
    }
    
}

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    num_threads_ = DEFAULT_NUM_THREADS;
    std::thread pool[num_threads_];
    SpinningWorkerArgs args[num_threads_];
    PoolAssignment assignments[num_threads_] = {0, nullptr, 0, 0, 0};
    pool_ = pool;
    assignments_ = assignments;
    finished_ = false;
    still_running_ = 0;



    for (int i = 0; i < num_threads_; i++) {
        args[i] = {i, assignments_, &finished_, &mutex_, &still_running_};
        pool_[i] = std::thread(parallelSpawnWorkerThreadSpinning, args + i);
    }
    
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {
    finished_ = true;
    for (int i = 0; i < num_threads_; i++) {
        pool_[i].join();
    }
}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    // for (int i = 0; i < num_total_tasks; i++) {
    //     runnable->runTask(i, num_total_tasks);
    // }

    int tasks_per_thread = (num_total_tasks + num_threads_ - 1) / num_threads_;
    for (int i = 0; i < num_total_tasks; i += tasks_per_thread) {
        mutex_.lock();
        still_running_ += 1;
        mutex_.unlock();
        assignments_[i / tasks_per_thread] = {true, runnable, i, std::min(num_total_tasks, i + tasks_per_thread), num_total_tasks};
    }
    while (still_running_ > 0) {}
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable* runnable, int num_total_tasks,
                                                              const std::vector<TaskID>& deps) {
    // You do not need to implement this method.
    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync() {
    // You do not need to implement this method.
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

TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(int num_threads): ITaskSystem(num_threads) {
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping() {
    //
    // TODO: CS149 student implementations may decide to perform cleanup
    // operations (such as thread pool shutdown construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable* runnable, int num_total_tasks) {


    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    for (int i = 0; i < num_total_tasks; i++) {
        runnable->runTask(i, num_total_tasks);
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
