#include "tasksys.h"

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
    for (int i = 0; i < num_total_tasks; i++)
    {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemSerial::sync()
{
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
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
}

TaskSystemParallelSpawn::~TaskSystemParallelSpawn() {}

void TaskSystemParallelSpawn::run(IRunnable *runnable, int num_total_tasks)
{
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++)
    {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelSpawn::runAsyncWithDeps(IRunnable *runnable, int num_total_tasks,
                                                 const std::vector<TaskID> &deps)
{
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
    for (int i = 0; i < num_total_tasks; i++)
    {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelSpawn::sync()
{
    // NOTE: CS149 students are not expected to implement TaskSystemParallelSpawn in Part B.
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

TaskSystemParallelThreadPoolSpinning::TaskSystemParallelThreadPoolSpinning(int num_threads) : ITaskSystem(num_threads)
{
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
}

TaskSystemParallelThreadPoolSpinning::~TaskSystemParallelThreadPoolSpinning() {}

void TaskSystemParallelThreadPoolSpinning::run(IRunnable *runnable, int num_total_tasks)
{
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    for (int i = 0; i < num_total_tasks; i++)
    {
        runnable->runTask(i, num_total_tasks);
    }
}

TaskID TaskSystemParallelThreadPoolSpinning::runAsyncWithDeps(IRunnable *runnable, int num_total_tasks,
                                                              const std::vector<TaskID> &deps)
{
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
    for (int i = 0; i < num_total_tasks; i++)
    {
        runnable->runTask(i, num_total_tasks);
    }

    return 0;
}

void TaskSystemParallelThreadPoolSpinning::sync()
{
    // NOTE: CS149 students are not expected to implement TaskSystemParallelThreadPoolSpinning in Part B.
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
    next_BulkTask_id_ = 0;

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
    ready_q_cv_.notify_all();
    for (int i = 0; i < num_threads_; i++)
    {
        pool_[i].join();
    }
}

void TaskSystemParallelThreadPoolSleeping::add_tasks_ready_q(IRunnable *runnable, int num_total_tasks, TaskID BulkTask_id)
{
    ready_q_mutex_.lock();
    for (int i = 0; i < num_total_tasks; i++)
    {
        Task task = {runnable, i, num_total_tasks, BulkTask_id};
        ready_q_.push(task);
    }
    ready_q_mutex_.unlock();
    ready_q_cv_.notify_all();
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable *runnable, int num_total_tasks)
{

    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //

    task_completed_ = 0;
    add_tasks_ready_q(runnable, num_total_tasks, 0);

    while (true)
    {
        Task cur_task;
        // Lock and wait on Queue
        { // Start of ready_q_mutex_ lock scope
            // Lock on Queue
            std::unique_lock<std::mutex> lck(ready_q_mutex_);
            if (ready_q_.size() > 0)
            {
                cur_task = ready_q_.front();
                ready_q_.pop();
            }
            else
            {
                break;
            }
        } // End of ready_q_mutex_ lock scope

        cur_task.runnable->runTask(cur_task.task_index, cur_task.num_total_tasks);
        task_completed_mutex_.lock();
        task_completed_ += 1;
        task_completed_mutex_.unlock();
    }

    std::unique_lock<std::mutex> lck(task_completed_mutex_);
    task_completed_cv_.wait(lck, [this, num_total_tasks]
                            { return this->task_completed_ == num_total_tasks; });
}

// void TaskSystemParallelThreadPoolSleeping::run(IRunnable *runnable, int num_total_tasks)
// {

//     //
//     // TODO: CS149 students will modify the implementation of this
//     // method in Parts A and B.  The implementation provided below runs all
//     // tasks sequentially on the calling thread.
//     //

//     std::vector<TaskID> deps;
//     TaskID task_id = runAsyncWithDeps(runnable, num_total_tasks, deps);
//     sync();
// }

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable *runnable, int num_total_tasks,
                                                              const std::vector<TaskID> &deps)
{

    //
    // TODO: CS149 students will implement this method in Part B.
    //

    TaskID current_BulkTask_id;
    bool new_tasks_scheduled = false;

    // Start of BulkTask_mutex_ lock scope
    {
        std::unique_lock<std::mutex> lck(BulkTask_mutex_);
        current_BulkTask_id = next_BulkTask_id_;
        next_BulkTask_id_ += 1;

        // Increment total_tasks_
        task_completed_mutex_.lock();
        total_tasks_ += num_total_tasks;
        task_completed_mutex_.unlock();
        // printf("TaskID %d, Total tasks %d\n", current_BulkTask_id, total_tasks_);

        // Track BulkTask
        BulkTask task;
        BulkTask_lookup_[current_BulkTask_id] = task;
        BulkTask_lookup_[current_BulkTask_id].runnable = runnable;
        BulkTask_lookup_[current_BulkTask_id].num_total_tasks = num_total_tasks;

        // Populate children
        for (TaskID dep : deps)
        {
            BulkTask_children_[dep].push_back(current_BulkTask_id);
        }

        // Mark scheduled as false.
        BulkTask_scheduled_[current_BulkTask_id] = false;

        // If no deps, Add them to queue right away.
        if (deps.size() == 0)
        {
            new_tasks_scheduled = true;
            // Add tasks to ready queue. It acquires a lock inside add_tasks_ready_q
            add_tasks_ready_q(BulkTask_lookup_[current_BulkTask_id].runnable, BulkTask_lookup_[current_BulkTask_id].num_total_tasks, current_BulkTask_id);
            // printf("TaskID %d, adding tasks to readyQueue\n", current_BulkTask_id);

            // Mark task as scheduled
            BulkTask_scheduled_[current_BulkTask_id] = true;
        }
    }
    // End of BulkTask_mutex_ lock scope

    if (new_tasks_scheduled == true)
    {
        // Signal workers.
        ready_q_cv_.notify_all();
    }

    return current_BulkTask_id;
}

void TaskSystemParallelThreadPoolSleeping::sync()
{
    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //

    // Do some WORK!
    while (true)
    {
        Task cur_task;
        // Lock and wait on Queue
        { // Start of ready_q_mutex_ lock scope
            // Lock on Queue
            std::unique_lock<std::mutex> lck(ready_q_mutex_);
            if (ready_q_.size() > 0)
            {
                cur_task = ready_q_.front();
                ready_q_.pop();
            }
            else
            {
                break;
            }
        } // End of ready_q_mutex_ lock scope

        cur_task.runnable->runTask(cur_task.task_index, cur_task.num_total_tasks);
        task_completed_mutex_.lock();
        task_completed_ += 1;
        task_completed_mutex_.unlock();
    }

    std::unique_lock<std::mutex> lck(task_completed_mutex_);
    task_completed_cv_.wait(lck, [this]
                            { return this->task_completed_ == this->total_tasks_; });
    return;
}

void TaskSystemParallelThreadPoolSleeping::parallelSpawnWorkerThreadSleeping(int id)
{
    while (!finished_)
    {
        Task cur_task;
        bool assigned = false;
        bool new_tasks_scheduled = true;

        // Lock and wait on Queue
        { // Start of ready_q_mutex_ lock scope
            // Lock on Queue
            std::unique_lock<std::mutex> lck(ready_q_mutex_);
            // Wait till a) Queue has something OR b) finished_
            ready_q_cv_.wait(lck, [this]
                             { return !ready_q_.empty() || finished_; });
            // Return if a) Queue is empty AND b) finished
            if (finished_ && ready_q_.empty())
            {
                return;
            }
            // We have some work to do
            if (ready_q_.size() > 0)
            {
                cur_task = ready_q_.front();
                ready_q_.pop();
                assigned = true;
                // std::cout << "thread " << id << " running task " << cur_task.BulkTask_id << "." << cur_task.task_index << "\n";
            }
        } // End of ready_q_mutex_ lock scope
        ready_q_cv_.notify_one(); // Gavin thinks it is not necessary

        if (assigned)
        {
            cur_task.runnable->runTask(cur_task.task_index, cur_task.num_total_tasks);
            task_completed_mutex_.lock();
            task_completed_ += 1;

            if (task_completed_ == total_tasks_)
            {
                task_completed_mutex_.unlock();
                task_completed_cv_.notify_one();
            }
            else
            {
                task_completed_mutex_.unlock();
            }
        }

        // Only Thread0 Needs to do this
        if (id % num_threads_ == 0)
        { // Scope for BulkTask_mutex_

            std::unique_lock<std::mutex> lck(BulkTask_mutex_);
            // Check if BulkTask_ID's children are worth executing.
            for (TaskID child : BulkTask_children_[cur_task.BulkTask_id])
            {
                if (BulkTask_scheduled_[child] == false)
                {
                    new_tasks_scheduled = true;
                    // Add tasks to ready queue. It acquires a lock inside add_tasks_ready_q
                    add_tasks_ready_q(BulkTask_lookup_[child].runnable, BulkTask_lookup_[child].num_total_tasks, child);

                    // Mark task as scheduled
                    BulkTask_scheduled_[child] = true;
                }
            }
            if (new_tasks_scheduled == true)
            {
                // Signal workers.
                ready_q_cv_.notify_all();
            }
        } // Scope for BulkTask_mutex_
    }
}
