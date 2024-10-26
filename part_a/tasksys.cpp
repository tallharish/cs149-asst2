#include "tasksys.h"
#include "iostream"

IRunnable::~IRunnable() {}

ITaskSystem::ITaskSystem(int num_threads) {}
ITaskSystem::~ITaskSystem() {}

/*
 * ================================================================
 * Serial task system implementation
 * ================================================================
 */

const char *TaskSystemSerial::name()
{
    return "Serial";
}

TaskSystemSerial::TaskSystemSerial(int num_threads) : ITaskSystem(num_threads)
{
}

TaskSystemSerial::~TaskSystemSerial() {}

void TaskSystemSerial::run(IRunnable *runnable, int num_total_tasks)
{
    for (int i = 0; i < num_total_tasks; i++)
    {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemSerial::runAsyncWithDeps(IRunnable *runnable, int num_total_tasks,
                                          const std::vector<TaskID> &deps)
{
    // You do not need to implement this method.
    return 0;
}

void TaskSystemSerial::sync()
{
    // You do not need to implement this method.
    return;
}

/*
 * ================================================================
 * Parallel Task System Implementation
 * ================================================================
 */

const char *TaskSystemParallelSpawn::name()
{
    return "Parallel + Always Spawn";
}

TaskSystemParallelSpawn::TaskSystemParallelSpawn(int num_threads) : ITaskSystem(num_threads)
{
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    parallel_threads = num_threads;
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

// Part a.1 - v1
void parallelSpawnWorkerThread(ParallelSpawnWorkerArgs *const args)
{
    for (int i = args->task_start_index; i < args->task_end_index; i++)
    {
        args->runnable->runTask(i, args->num_total_tasks);
    }
}

// Part a.1 - v2
void parallelSpawnSingleTaskThread(ParallelSpawnWorkerArgs *const args)
{
    args->runnable->runTask(args->task_start_index, args->num_total_tasks);
}

// Part a.1 - v3
void parallelSpawnSingleTaskThreadQueue(ParallelSpawnWorkerArgs *const args)
{
    int local_task_index;
    while (true)
    {
        // Acquire task_mutex;
        args->task_mutex->lock();
        // read task_index; assign to local_task_index, increment task_index;
        local_task_index = *args->task_index;
        *args->task_index += 1;
        // Release task_mutex;
        args->task_mutex->unlock();
        if (local_task_index >= args->num_total_tasks)
        {
            break;
        }
        else
        {
            args->runnable->runTask(local_task_index, args->num_total_tasks);
        }
    }
}

/*
// Part a.1 - v1
void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {
    // v1 - baseline implementation - static assignment
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.

    int tasks_per_thread = (num_total_tasks + parallel_threads - 1) / parallel_threads;
    for (int i = 0; i < num_total_tasks; i += tasks_per_thread) {
        workerArgs[i / tasks_per_thread] = {runnable, i, std::min(num_total_tasks, i + tasks_per_thread), num_total_tasks};
        workers.push_back(std::thread(parallelSpawnWorkerThread, &workerArgs[i / tasks_per_thread]));
    }

    for (std::thread& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

// Part a.1 - v2
void TaskSystemParallelSpawn::run(IRunnable* runnable, int num_total_tasks) {

    // v2 - static assignment of one task per thread iteratively
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.

    int task_index = 0;
    int iteration_index = 0;
    while (task_index < num_total_tasks) {
        for (int j = 0; j < std::min(parallel_threads, (num_total_tasks - iteration_index*parallel_threads)); j += 1) {
            workerArgs[j] = {runnable, task_index, task_index, num_total_tasks};
            workers.push_back(std::thread(parallelSpawnSingleTaskThread, &workerArgs[j]));
            std::cout << "Assigning task " << task_index << " to thread %d" << j << " worker size " << workers.size() << std::endl;
            //printf(" %d to thread %d, vector size %d", task_index, j, workers.size());
            task_index += 1;
        }
        iteration_index += 1;
        for (std::thread& worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        workers.clear();
    }
}
*/

// Part a.1 - v3
void TaskSystemParallelSpawn::run(IRunnable *runnable, int num_total_tasks)
{

    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    int *task_index = new int;
    std::mutex *task_mutex = new std::mutex;
    *task_index = 0;
    for (int j = 0; j < parallel_threads; j += 1)
    {
        workerArgs[j] = {runnable, -1, -1, num_total_tasks, task_mutex, task_index};
        workers.push_back(std::thread(parallelSpawnSingleTaskThreadQueue, &workerArgs[j]));
    }
    for (std::thread &worker : workers)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
    workers.clear();
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable *runnable, int num_total_tasks,
                                                 const std::vector<TaskID> &deps)
{
    // You do not need to implement this method.
    return 0;
}

void TaskSystemParallelSpawn::sync()
{
    // You do not need to implement this method.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Spinning Task System Implementation
 * ================================================================
 */

const char *TaskSystemParallelThreadPoolSpinning::name()
{
    return "Parallel + Thread Pool + Spin";
}

void TaskSystemParallelThreadPoolSpinning::parallelSpawnWorkerThreadSpinning(int id)
{
    // finished_ is set to true when the deconstructor is called - this tells the thread to stop looping and exit
    while (!finished_)
    {
        Task cur_task;
        bool assigned = false;

        task_q_mutex_.lock();
        // check if there is any unassigned tasks from the queue; if yes, current thread is assigned to complete the
        // first take in queue
        if (unassigned_tasks_.size() > 0)
        {
            cur_task = unassigned_tasks_.front();
            unassigned_tasks_.pop();
            assigned = true;
        }
        task_q_mutex_.unlock();
        if (assigned)
        {
            cur_task.runnable->runTask(cur_task.task_index, cur_task.num_total_tasks);
            assigned = false;
            // upon completing task, increment num_completed_
            // when num_completed_ == num_total_tasks, the run function exits
            num_completed_mutex_.lock();
            num_completed_ += 1;
            num_completed_mutex_.unlock();
        }
    }
}

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads) : ITaskSystem(num_threads)
{
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    num_threads_ = num_threads - 1;
    finished_ = false;
    // create threads and push to vector
    for (int i = 0; i < num_threads_; i++)
    {
        pool_.push_back(std::thread(&TaskSystemParallelThreadPoolSpinning::parallelSpawnWorkerThreadSpinning, this, i));
    }
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning()
{
    finished_ = true; // this tells the thread function to stop looping and exit
    for (int i = 0; i < num_threads_; i++)
    {
        pool_[i].join();
    }
}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable *runnable, int num_total_tasks)
{
    //
    // TODO: CS149 students will modify the implementation of this
    // method in Part A.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    num_completed_ = 0;
    task_q_mutex_.lock();
    for (int i = 0; i < num_total_tasks; i++)
    {
        Task task = {runnable, i, num_total_tasks};
        unassigned_tasks_.push(task);
    }
    task_q_mutex_.unlock();

    /* Following code ensures that run() returns only when all tasks are complete*/
    while (true)
    {
        Task cur_task;
        task_q_mutex_.lock();
        // check if there is any unassigned tasks from the queue; if yes, current thread is assigned to complete the
        // first take in queue
        if (unassigned_tasks_.size() > 0)
        {
            cur_task = unassigned_tasks_.front();
            unassigned_tasks_.pop();
            task_q_mutex_.unlock();
        }
        else
        {
            task_q_mutex_.unlock();
            break;
        }

        cur_task.runnable->runTask(cur_task.task_index, cur_task.num_total_tasks);
        // upon completing task, increment num_completed_
        num_completed_mutex_.lock();
        num_completed_ += 1;
        num_completed_mutex_.unlock();
    }

    while (true)
    {
        num_completed_mutex_.lock();
        if (num_completed_ == num_total_tasks)
        {
            num_completed_mutex_.unlock();
            break;
        }
        num_completed_mutex_.unlock();
    }
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable *runnable, int num_total_tasks,
                                                              const std::vector<TaskID> &deps)
{
    // You do not need to implement this method.
    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync()
{
    // You do not need to implement this method.
    return;
}

/*
 * ================================================================
 * Parallel Thread Pool Sleeping Task System Implementation
 * ================================================================
 */

const char *TaskSystemParallelThreadPoolSleeping::name()
{
    return "Parallel + Thread Pool + Sleep";
}

void TaskSystemParallelThreadPoolSleeping::parallelSpawnWorkerThreadSleeping(int id)
{
    while (!finished_)
    {
        Task cur_task;
        bool assigned = false;

        // Lock and wait on Queue
        { // Start of task_q_mutex_ lock scope
            // Lock on Queue
            std::unique_lock<std::mutex> lck(task_q_mutex_);
            // Wait till a) Queue has something OR b) finished_
            task_q_cv_.wait(lck, [this]
                            { return !unassigned_tasks_.empty() || finished_; });
            // Return if a) Queue is empty AND b) finished
            if (finished_ && unassigned_tasks_.empty())
            {
                return;
            }
            // We have some work to do
            if (unassigned_tasks_.size() > 0)
            {
                cur_task = unassigned_tasks_.front();
                unassigned_tasks_.pop();
                assigned = true;
            }
        } // End of task_q_mutex_ lock scope

        if (assigned)
        {
            cur_task.runnable->runTask(cur_task.task_index, cur_task.num_total_tasks);
            task_q_mutex_.lock();
            num_completed_ += 1;
            if (num_completed_ == cur_task.num_total_tasks)
            {
                task_q_mutex_.unlock();
                task_q_cv_.notify_all();
            }
            else
            {
                task_q_mutex_.unlock();
            }
        }
    }
}

TaskSystemParallelThreadPoolSleeping::TaskSystemParallelThreadPoolSleeping(int num_threads) : ITaskSystem(num_threads)
{
    //
    // TODO: CS149 student implementations may decide to perform setup
    // operations (such as thread pool construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    num_threads_ = num_threads - 1;
    finished_ = false;

    for (int i = 0; i < num_threads_; i++)
    {
        pool_.push_back(std::thread(&TaskSystemParallelThreadPoolSleeping::parallelSpawnWorkerThreadSleeping, this, i));
    }
}

TaskSystemParallelThreadPoolSleeping::~TaskSystemParallelThreadPoolSleeping()
{
    //
    // TODO: CS149 student implementations may decide to perform cleanup
    // operations (such as thread pool shutdown construction) here.
    // Implementations are free to add new class member variables
    // (requiring changes to tasksys.h).
    //
    finished_ = true;
    task_q_cv_.notify_all();
    for (int i = 0; i < num_threads_; i++)
    {
        pool_[i].join();
    }
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable *runnable, int num_total_tasks)
{

    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    num_completed_ = 0;
    task_q_mutex_.lock();
    for (int i = 0; i < num_total_tasks; i++)
    {
        Task task = {runnable, i, num_total_tasks};
        unassigned_tasks_.push(task);
    }
    task_q_mutex_.unlock();
    task_q_cv_.notify_all();

    while (true)
    {
        Task cur_task;
        bool assigned = false;

        // Lock and wait on Queue
        { // Start of task_q_mutex_ lock scope
            // Lock on Queue
            std::unique_lock<std::mutex> lck(task_q_mutex_);
            // Wait till a) Queue has something OR b) finished_
            task_q_cv_.wait(lck, [this, num_total_tasks]
                            { return !unassigned_tasks_.empty() || (num_completed_ == num_total_tasks); });

            // We are done
            if (num_completed_ == num_total_tasks)
            {
                break;
            }
            // We have some work to do
            if (unassigned_tasks_.size() > 0)
            {
                cur_task = unassigned_tasks_.front();
                unassigned_tasks_.pop();
                assigned = true;
            }
        } // End of task_q_mutex_ lock scope

        if (assigned)
        {
            cur_task.runnable->runTask(cur_task.task_index, cur_task.num_total_tasks);
            task_q_mutex_.lock();
            num_completed_ += 1;
            task_q_mutex_.unlock();
        }
    }

    // std::unique_lock<std::mutex> lck(num_completed_mutex_);
    // num_completed_cv_.wait(lck, [this, num_total_tasks]
    //                        { return this->num_completed_ == num_total_tasks; });
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable *runnable, int num_total_tasks,
                                                              const std::vector<TaskID> &deps)
{

    //
    // TODO: CS149 students will implement this method in Part B.
    //

    return 0;
}

void TaskSystemParallelThreadPoolSleeping::sync()
{

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    return;
}
