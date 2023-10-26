#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "psched.h"
#include "spinlock.h"

// Needed for sys_sleep() ptable lock
extern struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  // uint ticks0;
  uint endticks;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&ptable.lock);
  
  endticks = ticks + n;
  myproc()->endticks = endticks;

  while(ticks < endticks) {
    if(myproc()->killed) {
      release(&ptable.lock);
      return -1;
    }
    sleep(&ticks, &ptable.lock);  // Sleep until the entire duration has passed
  }

  release(&ptable.lock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// Lowers priority of a process
// Allows a process to voluntarily decrease its priority
int sys_nice(void) {
    int n;

    // Get the argument from the stack, return -1 if no arg
    if (argint(0, &n) < 0)
        return -1;

    // Check if the argument is valid
    if (n < 0 || n > 20)
        return -1;

    struct proc *curproc = myproc();
    int prev_nice = curproc->nice;
    curproc->nice = n;

    return prev_nice;
}

// Returns relevant scheduler-related information about all running processes, 
// including how many times each process has been chosen to run (ticks) 
// and the process ID of each process.
int sys_getschedstate(void)
{
  struct pschedinfo *currSched;

  // Get the argument from the stack, return -1 if no arg
  if(argptr(0, (void*)&currSched, sizeof(*currSched)) < 0 || !currSched)
    return -1;

  acquire(&ptable.lock);

  // Loop through all processes and populate the pschedinfo struct
  for(int i = 0; i < NPROC; i++) {
    struct proc *p = &ptable.proc[i];
    currSched->inuse[i] = (p->state != UNUSED);
    currSched->priority[i] = p->priority;
    currSched->nice[i] = p->nice;
    currSched->pid[i] = p->pid;
    currSched->ticks[i] = p->cpu;
  }
  
  release(&ptable.lock);

  return 0;
}