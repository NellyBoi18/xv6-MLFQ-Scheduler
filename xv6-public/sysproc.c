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
  /*
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  */
  
  
  endticks = ticks + n;
  myproc()->endticks = endticks;

  while(ticks < endticks) {
    if(myproc()->killed) {
      release(&ptable.lock);
      return -1;
    }
    sleep(&ticks, &ptable.lock);  // Sleep until the entire duration has passed
  }
  

  // acquire(&ptable.lock);
  /*
  myproc()->cpu = n;
  myproc()->state = SLEEPING;
  sched();
  */
  // release(&ptable.lock);

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
int sys_nice(void) {
    int n;
    if (argint(0, &n) < 0)
        return -1;

    if (n < 0 || n > 20)
        return -1;

    struct proc *curproc = myproc();
    int prev_nice = curproc->nice;
    curproc->nice = n;

    return prev_nice;
}

// Returns the current priority of a process
/*
int sys_getschedstate(void) {
    struct pschedinfo *pinfo;
    if (argptr(0, (void *)&pinfo, sizeof(*pinfo)) < 0)
        return -1;

    for (struct proc *p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        pinfo->inuse[p - ptable.proc] = p->state != UNUSED;
        pinfo->priority[p - ptable.proc] = p->priority;
        pinfo->nice[p - ptable.proc] = p->nice;
        pinfo->pid[p - ptable.proc] = p->pid;
        pinfo->ticks[p - ptable.proc] = p->cpu;
    }

    return 0;
}
*/

int sys_getschedstate(void)
{
  struct pschedinfo *psi;

  if(argptr(0, (void*)&psi, sizeof(*psi)) < 0 || !psi)
    return -1;

  acquire(&ptable.lock);
  for(int i = 0; i < NPROC; i++) {
    struct proc *p = &ptable.proc[i];
    psi->inuse[i] = (p->state != UNUSED);
    psi->priority[i] = p->priority;
    psi->nice[i] = p->nice;
    psi->pid[i] = p->pid;
    psi->ticks[i] = p->cpu;  // Assuming the 'cpu' field in proc struct represents ticks. Adjust if needed.
  }
  release(&ptable.lock);

  return 0;
}