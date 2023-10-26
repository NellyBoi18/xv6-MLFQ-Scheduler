# xv6 MLFQ Scheduler

Implemented a multi-level feedback queue scheduler with cpu decay for xv6 in this project. Process priorities will be set using their prior cpu usage (details below), and processes which have used less cpu recently will have higher priority. This is a different MLFQ scheduler than presented in OSTEP, but the principle of adjusting priority based on prior cpu usage is the same. Also implemented a `nice()` system call, which lowers the priority of a process. On every tick, the scheduler will schedule the runnable process with the highest priority, or round-robin amongst processes with the same priority.

# Project details

## MLFQ Scheduler

xv6 uses a simple round-robin scheduler. In this project, I modified the scheduler to schedule processes according to process priority. Priority is function of recent cpu usage. The kernel keeps track of cpu usage, by counting the number of ticks the process has spent running, for each process. To reward processes which have not used the cpu recently, the total cpu usage for each process is "decayed" when priorities are recalculated. Our decay function is simply to divide cpu usage in half.

In my MLFQ scheduler, process priorities are recalculated every second. The following method is used to determine priority:

``` example
def decay(cpu):
    return cpu/2
    
cpu = decay(cpu)
priority = cpu/2 + nice
```

For new processes, priority, nice, and cpu values should all be set to zero. **Higher priority values correspond to lower process priority, lower values correspond to higher process priority**.

### A note about xv6 timing

A `tick` in xv6 represents about 10ms of time. Thus, priorities are recalculated every 100 ticks. In both the default xv6 scheduler and mine, the scheduler runs on every tick. In other words, a high-priority process will `yield()` on every tick, but may be rescheduled if it is still the highest-priority process on the system.

### Modified `sleep()` system call
The behavior of the xv6 `sleep()` system call interferes with my scheduler. `sleep(n)` puts a process to sleep for `n` ticks. On every timer interrupt, xv6 wakes up all sleeping processes. If the process still has more ticks to sleep, it goes back to sleep until the next timer interrupt when the process repeats.

I modified the `sleep()` system call so that processes are only runnable after all of their sleeping ticks have expired.

## New System Calls

I created two new system calls to implement this scheduler. The first is a `nice()` system call which allows a process to lower its priority, and the second is `getschedstate()` which returns some information about priorities and ticks to the user. 

### `nice()` system call

The `nice()` system call allows a process to voluntarily decrease its priority. The nice value is added to a process's priority. You should implement a system call with the following prototype

``` c
int nice(int n);
```

`nice()` should set the current process's nice value to `n` and return the previous nice value, or -1 on error. Nice values lower than 0 or greater than 20 are considered invalid and the system call should return an error if a user attempts to set a nice value outside these bounds. 

### `getschedstate()` system call

This routine (detailed below) returns relevant scheduler-related information about all running processes, including how many times each process has been chosen to run (ticks) and the process ID of each process. The structure `pschedinfo` is defined below. This routine should return 0 if successful, and -1 otherwise (if, for example, a bad or NULL pointer is passed into the kernel).

``` c
int getschedstate(struct pschedinfo *);
```

A new file called `psched.h` was created for this struct and the code is below.

```c
#ifndef _PSCHED_H_
#define _PSCHED_H_

#include "param.h"

struct pschedinfo {
  int inuse[NPROC];    // whether this slot of the process table is in use (1 or 0)
  int priority[NPROC]; // the priority of each process
  int nice[NPROC];     // the nice value of each process 
  int pid[NPROC];      // the PID of each process 
  int ticks[NPROC];    // the number of ticks each process has accumulated 
};

#endif // _PSCHED_H_
```

### `Makefile` changes
In order to test the changes, two changes were made to the xv6 `Makefile`. `CPUS := 1`, as we are only considering scheduling on a single core. I also disabled compiler optimizations by changing the `CFLAGS` from `-O2` to `-O0`. Some of the tests use spin loops, which may be removed by the compiler if optimizations are turned on.