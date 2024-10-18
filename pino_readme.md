Pino updates:

work so far on part a, step 2: code can be found in branch part_a_2_pino

**in the constructor:**
-array of threads, called pool_
-array of PoolAssignments, called assignments_, one per thread:
  - PoolAssignment structs keep track of work assigned to each thread
- finished_
  - bool, initially set to false. deconstructor sets this to true and is an indication for the spinning threads to stop
- still_running_
  - keeps track of number of threads that still has not finished its assigned work
  - this is a SHARED resource: each thread decrements still_running_ when it finishes its assigned load
- mutex_
  - currently used to prevent multiple threads changing still_running_ at the same time

**in run**
- does static assignment of loads, similar to in step 1
- assigns a new load to a thread by updating assignments_, and increments still_running_ by 1
