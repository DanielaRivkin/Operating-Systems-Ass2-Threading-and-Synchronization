#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    acquire(&mykthread()->kthreadLock); 
    int killcheck = mykthread()->killed;
    release(&mykthread()->kthreadLock);
    if(killed(myproc())||killcheck){ // added a check if the kthread is killed
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64 sys_kthread_create(void){
  uint64 func;
  uint64 stack;
  int size;
  argaddr(0,&func);
  argaddr(1,&stack);
  argint(2, &size);
  return kthread_create(func,stack,size);
}

uint64 sys_kthread_id(void){
    return mykthread()->threadID;
}

uint64 sys_kthread_kill(void){
  int id;
  argint(0,&id);
  return kthread_kill(id);
}
uint64 sys_kthread_exit(void){
  int n;
  argint(0,&n);
  kthread_exit(n);
  return 0;
}

uint64 sys_kthread_join(void){
  int id;
  uint64 n;
  argint(0,&id);
  argaddr(1,&n);
  kthread_join(id,(int*)n);
  return 0;
}