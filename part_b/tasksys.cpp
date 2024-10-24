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
            assigned = false;
            num_completed_mutex_.lock();
            num_completed_ += 1;
            if (num_completed_ == cur_task.num_total_tasks)
            {
                num_completed_cv_.notify_one();
            }
            num_completed_mutex_.unlock();
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
    num_threads_ = num_threads;
    finished_ = false;
    next_task_id_ = 0;

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

void TaskSystemParallelThreadPoolSleeping::reset_num_completed()
{
    num_completed_mutex_.lock();
    num_completed_ = 0;
    num_completed_mutex_.unlock();
}

void TaskSystemParallelThreadPoolSleeping::add_tasks_queue(IRunnable *runnable, int num_total_tasks)
{
    task_q_mutex_.lock();
    for (int i = 0; i < num_total_tasks; i++)
    {
        Task task = {runnable, i, num_total_tasks};
        unassigned_tasks_.push(task);
    }
    task_q_mutex_.unlock();
    task_q_cv_.notify_all();
}

void TaskSystemParallelThreadPoolSleeping::wait_unassigned_tasks(int num_total_tasks)
{
    std::unique_lock<std::mutex> lck(num_completed_mutex_);
    num_completed_cv_.wait(lck, [this, num_total_tasks]
                           { return this->num_completed_ == num_total_tasks; });
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable *runnable, int num_total_tasks)
{

    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    reset_num_completed();
    add_tasks_queue(runnable, num_total_tasks);
    wait_unassigned_tasks(num_total_tasks); // Blocking call.
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable *runnable, int num_total_tasks,
                                                              const std::vector<TaskID> &deps)
{

    //
    // TODO: CS149 students will implement this method in Part B.
    //
    task_management_mutex_.lock();
    // Assign current task_id
    TaskID cur_task_id = next_task_id_;
    next_task_id_ += 1; // increment next_task_id in prep for next call to runAsyncWithDeps

    // Populate Parent Deps
    std::unordered_set<TaskID> parent_set;
    parent_[cur_task_id] = parent_set;
    for (TaskID dep : deps)
    {
        parent_[cur_task_id].insert(dep);
    }

    // Populate # of parents
    num_incomplete_parents_[cur_task_id] = deps.size();

    // Mark task as incomplete
    task_completed_[cur_task_id] = false;

    // Track Task_group
    Task_group task;
    task_lookup_[cur_task_id] = task;
    task_lookup_[cur_task_id].runnable = runnable;
    task_lookup_[cur_task_id].num_total_tasks = num_total_tasks;

    // Add to pending tasks
    // printf("\n Inserting %d", cur_task_id);
    pending_tasks_.insert(cur_task_id);
    task_management_mutex_.unlock();

    return cur_task_id;
}

void TaskSystemParallelThreadPoolSleeping::sync()
{

    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //
    task_management_mutex_.lock();
    std::unordered_set<TaskID> next_tasks_;
    int next_total_tasks_;

    while (!pending_tasks_.empty())
    {
        next_total_tasks_ = 0;
        // Iterate through tasks that are incomplete and have no parents -> add to next_tasks_
        for (TaskID pend_task : pending_tasks_)
        {
            if (task_completed_[pend_task] == false && num_incomplete_parents_[pend_task] == 0)
            {
                next_tasks_.insert(pend_task);
            }
            // more conditions ??
        }
        // Iterate through next_tasks_ and add jobs to task_queue
        reset_num_completed();
        for (TaskID next_task : next_tasks_)
        {
            // printf("\n next_task %d - %d tasks", next_task, task_lookup_[next_task].num_total_tasks);
            add_tasks_queue(task_lookup_[next_task].runnable, task_lookup_[next_task].num_total_tasks);
            next_total_tasks_ += task_lookup_[next_task].num_total_tasks;
        }

        // Wait for all pending jobs to finish
        wait_unassigned_tasks(next_total_tasks_);

        // Update book-keeping
        //      Lookup parent_ and update the num_incomplete_parents_ for those tasks.
        for (TaskID pend_task : pending_tasks_)
        {
            for (TaskID next_task : next_tasks_)
            {
                // If next_task_ in parent_[pend_task] => reduce
                auto it = parent_[pend_task].find(next_task);
                // next_task is one of the parents
                if (it != parent_[pend_task].end())
                {

                    num_incomplete_parents_[pend_task] -= 1;
                }
            }
        }
        //      Update task_completed_
        //      Remove all next_tasks_ from pending_tasks_
        for (TaskID next_task : next_tasks_)
        {
            task_completed_[next_task] = true;
            pending_tasks_.erase(next_task);
        }

        // Clear next_tasks_
        next_tasks_.clear();
    }

    task_management_mutex_.unlock();
    return;
}
