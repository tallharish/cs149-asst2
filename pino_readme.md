Pino updates (can be found in branch part_a_3_pino)

**part a step 2 is done**
- main implementation idea: maintain queue of tasks unassigned to threads
- each free thread checks if the task queue is empty; if not, it pops the queue and completes the front-most task
- one a thread finishes a task, it increments num_completed_; once num_completed_ is equal to num_total_tasks, run exits.

*passes all performance tests (usually) - there's some instability on myth so we may have to double check on aws

**part a step 3 - initial work**
- two areas of work:
 (a) run should be put to sleep until all tasks are completed
(b) each thread should be put to sleep if the task queue is empty

I believe (a) is done. However, the code I did for (b) runs into some sort of deadlock.

