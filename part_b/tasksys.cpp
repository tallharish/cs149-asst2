#include "tasksys.h"
#include <cassert>

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
    unscheduled_BulkTasks = 0;
    total_tasks_ = 0;
    task_completed_ = 0;
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

void TaskSystemParallelThreadPoolSleeping::add_tasks_ready_q(std::vector<Task> tasks)
{
    ready_q_mutex_.lock();
    for (Task task: tasks) {
        ready_q_.push(task);
    }
    ready_q_mutex_.unlock();
    ready_q_cv_.notify_all();
}

void TaskSystemParallelThreadPoolSleeping::on_task_complete(TaskID BulkTask_id) {
    // check if this is the last task in BulkTask; if it is, see if any new BulkTasks can be scheduled
    
    BulkTask_lookup_mutex_.lock();
    
    // std::cout << "inside update deps after lock" << std::endl;
    BulkTask_lookup_[BulkTask_id].num_tasks_completed += 1; // TODO: separate mutex per task??
    if (BulkTask_lookup_[BulkTask_id].num_tasks_completed == BulkTask_lookup_[BulkTask_id].num_total_tasks) {
        std::vector<TaskID> child_to_be_scheduled;

        // std::cout << "here1" << std::endl;
        
        BulkTask_lookup_mutex_.unlock();
        
        dep_mutex_.lock();
        // std::cout << "here2" << std::endl;
        BulkTask_completed_[BulkTask_id] = true;
        for (TaskID child: BulkTask_children_[BulkTask_id]) {
            num_incomplete_parents_[child] -= 1;
            if (num_incomplete_parents_[child] == 0) {
                child_to_be_scheduled.push_back(child);
            }
        }
        dep_mutex_.unlock();
        
        BulkTask_lookup_mutex_.lock();
        std::vector<Task> tasks = {};
        for (TaskID BulkTask_id: child_to_be_scheduled)
        {
            // std::cout << "inside add tasks ready q" << std::endl;
            for (int i = 0; i < BulkTask_lookup_[BulkTask_id].num_total_tasks; i++)
            {
                Task task = {BulkTask_lookup_[BulkTask_id].runnable, i, BulkTask_lookup_[BulkTask_id].num_total_tasks, BulkTask_id};
                tasks.push_back(task);
            }
        }
        BulkTask_lookup_mutex_.unlock();

        if (tasks.size() > 0) {
            add_tasks_ready_q(tasks);
            // BulkTask_scheduled_mutex_.lock();
            // unscheduled_BulkTasks -= child_to_be_scheduled.size();
            // BulkTask_scheduled_mutex_.unlock();
            task_completed_cv_.notify_one();// BulkTask_scheduled_cv_.notify_one();
        }
    }
    else 
    {
        BulkTask_lookup_mutex_.unlock();
    }
}

void TaskSystemParallelThreadPoolSleeping::run(IRunnable *runnable, int num_total_tasks)
{

    //
    // TODO: CS149 students will modify the implementation of this
    // method in Parts A and B.  The implementation provided below runs all
    // tasks sequentially on the calling thread.
    //
    std::vector<TaskID> empty_deps = {};
    runAsyncWithDeps(runnable, num_total_tasks, empty_deps);
    sync();
}

TaskID TaskSystemParallelThreadPoolSleeping::runAsyncWithDeps(IRunnable *runnable, int num_total_tasks,
                                                              const std::vector<TaskID> &deps)
{
    //
    // TODO: CS149 students will implement this method in Part B.
    //    
    TaskID current_BulkTask_id;
    
    // Start of BulkTask_mutex_ lock scope
    {
        // Increment total_tasks_
        task_completed_mutex_.lock(); 
        total_tasks_ += num_total_tasks;
        task_completed_mutex_.unlock();

        // std::unique_lock<std::mutex> lck(BulkTask_mutex_);
        
        current_BulkTask_id = next_BulkTask_id_;
        BulkTask_lookup_mutex_.lock();
        next_BulkTask_id_ += 1;

        // Track BulkTask
        BulkTask_lookup_[current_BulkTask_id] = {runnable, num_total_tasks, 0};
        BulkTask_lookup_mutex_.unlock();

        // Populate children, update num_incomplete_parents
        dep_mutex_.lock();
        int cur_incomplete_parents = 0;
        for (TaskID dep : deps)
        {
            BulkTask_children_[dep].push_back(current_BulkTask_id);
            cur_incomplete_parents += (!BulkTask_completed_[dep]);
        }
        num_incomplete_parents_[current_BulkTask_id] = cur_incomplete_parents;
        
        // Add new key to BulkTask_completed_
        BulkTask_completed_[current_BulkTask_id] = false;
        dep_mutex_.unlock();
        // ------check if current BulkTask ready to be scheduled ------   
            

        // If no incomplete deps, Add them to queue right away.
        if (cur_incomplete_parents == 0)
        {
            // Add tasks to ready queue. It acquires a lock inside add_tasks_ready_q
            ready_q_mutex_.lock();
            for (int i = 0; i < num_total_tasks; i++) 
            {
                ready_q_.push({runnable, i, num_total_tasks, current_BulkTask_id});
            }
            ready_q_mutex_.unlock();
            ready_q_cv_.notify_all();
        }
        // else 
        // {
        //     BulkTask_scheduled_mutex_.lock();
        //     unscheduled_BulkTasks += 1;
        //     BulkTask_scheduled_mutex_.unlock();
        // }
    }
    // End of BulkTask_mutex_ lock scope
    // std::cout << current_BulkTask_id << std::endl;
    // std::cout << "num task complete = " << task_completed_ << std::endl;

    return current_BulkTask_id;
}

void TaskSystemParallelThreadPoolSleeping::sync()
{
    //
    // TODO: CS149 students will modify the implementation of this method in Part B.
    //
    
    // TODO: need to fix this so sync continues to do work
    // Do some WORK!
    
    while (true) {
        // std::cout << task_completed_ << std::endl;
        
        std::unique_lock<std::mutex> lck(task_completed_mutex_);
        if (task_completed_ == total_tasks_) {
            break;
        }
        task_completed_cv_.wait(lck);
        
        // if (unscheduled_BulkTasks == 0) {
        //     break;
        // }
        //  BulkTask_scheduled_cv_.wait(lck);
        lck.unlock();
        while (true) {
            // std::cout << "line 353" << std::endl;
            Task cur_task;
            // Lock and wait on Queue
            { // Start of task_q_mutex_ lock scope
                // Lock on Queue
                std::unique_lock<std::mutex> lck(ready_q_mutex_);
                if (ready_q_.size() > 0)
                {
                    cur_task = ready_q_.front();
                    ready_q_.pop();
                }
                else {
                    break;
                }
            } // End of ready_q_mutex_ lock scope

            cur_task.runnable->runTask(cur_task.task_index, cur_task.num_total_tasks);
            task_completed_mutex_.lock();
            task_completed_ += 1;
            task_completed_mutex_.unlock();
            on_task_complete(cur_task.BulkTask_id);
           
        }
    }

    // std::unique_lock<std::mutex> lck(task_completed_mutex_);
    // task_completed_cv_.wait(lck, [this]
    //                         { return this->task_completed_ == this->total_tasks_; });
    // reset task counter just in case of wraparounds
    task_completed_ = 0;
    total_tasks_ = 0;
    return;
}

void TaskSystemParallelThreadPoolSleeping::parallelSpawnWorkerThreadSleeping(int id)
{
    while (!finished_)
    {
        Task cur_task;
        bool assigned = false;

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
            on_task_complete(cur_task.BulkTask_id); 
            
        }
    }
}
