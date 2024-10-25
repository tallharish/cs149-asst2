Pino updates (can be found in part_b_final)

**sync now does work**
- I'm using a double for loop;
- there is a condition variable in the outer loop that gets notified when
    (a) new tasks have been scheduled (thread should wake up to do work)
    (b) all tasks have been completed (thread should wake up to return)


**we are now keeping track of num_BulkTask_completed/total_BulkTasks instead of num_task_completed/total_tasks**
- this is so that we don't have to update it as often

**num_completed_mutex_ is being used to lock num_BulkTask_completed/total_BulkTasks**

**We now have two mutexes that help with BulkTask bookkeeping**
- BulkTask_lookup_mutex_ is used for all accesses to BulkTask_lookup struct
- dep_mutex_ is used for all accesses to structures that help track dependencies (children_, num_incomplete_parents_ etc.)
- *I'm not actually sure if this helps or hurts our program


**overall performance**
- we are passing everything except super super light (both run and runAsync).
- Making sync do work helped but it didn't give significant speedup. I think there's something major I'm missing here...



