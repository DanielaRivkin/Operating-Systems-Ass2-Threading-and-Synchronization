#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

extern void forkret(void);
extern struct proc proc[NPROC];

void kthreadinit(struct proc *p)
{
  initlock(&p->threadLock,"tlock");
  for (struct kthread *kt = p->kthread; kt < &p->kthread[NKT]; kt++)
  {
    initlock(&kt->kthreadLock,"ktlock");
    kt->state = threadUNUSED;
    kt->pcb = p;
    // WARNING: Don't change this line!
    // get the pointer to the kernel stack of the kthread
    kt->kstack = KSTACK((int)((p - proc) * NKT + (kt - p->kthread)));
  }
}

struct kthread *mykthread()
{
  push_off();
  struct cpu *c = mycpu();
  struct kthread * kt = c->kthread;
  pop_off();
  return kt;
}

struct trapframe *get_kthread_trapframe(struct proc *p, struct kthread *kt)
{
  return p->base_trapframes + ((int)(kt - p->kthread));
}

int alloctid(struct proc *p)
{ 
  int id;
  acquire(&p->threadLock);
  id = p->threadNum;
  p->threadNum++;
  release(&p->threadLock);
  return id;
}

struct kthread* allockthread(struct proc *p)
{ 
  struct kthread *kt;
  for(kt = p->kthread; kt < &p->kthread[NKT]; kt++)
  {
    acquire(&kt->kthreadLock);
    if(kt->state == threadUNUSED) {

      kt->threadID = alloctid(p);
      kt->state = USED;
      kt->trapframe = get_kthread_trapframe(p,kt);
      memset(&kt->context, 0, sizeof(kt->context));
      kt->context.ra = (uint64) forkret;
      kt->context.sp = kt->kstack + PGSIZE;
      return kt;

    } else {
      release(&kt->kthreadLock);
    }
  }
  return 0;
}

void freekthread(struct kthread *kt)
{
  kt->state = threadUNUSED; 
  kt->chan = 0;
  kt->killed = 0;
  kt->exitStatus = 0;
  kt->threadID = 0;
  kt->trapframe = 0;
}

int kthread_create(uint64 start_func_addr,uint64 stack , int stack_size) {
  struct kthread *t;
  if((t = allockthread(myproc())) != 0){
    t->trapframe->epc = start_func_addr;   
    t->trapframe->sp = stack + stack_size;
    t->state = threadRUNNABLE;
    release(&t->kthreadLock);
    return t->threadID; 
  }
  return -1;
}

int kthread_id()
{
  return mykthread()->threadID;
}

int kthread_kill(int id) {
  struct proc *p = myproc();
  for (int i = 0; i < NKT; i++) {
    struct kthread *t = &p->kthread[i];
    acquire(&t->kthreadLock);
    if (t->threadID == id) {
      if (t->state == threadSLEEPING) {
        t->state = threadRUNNABLE;
      }
      t->killed = 1;
      release(&t->kthreadLock);
      return 0;
    }
    release(&t->kthreadLock);
  }
  return -1;
}

