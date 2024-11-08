#ifndef __UTHREAD_H__
#define __UTHREAD_H__

#include <stdbool.h>

#define MAXULTHREADS 100

typedef enum ulthread_state
{
  FREE,
  RUNNABLE,
  YIELD,
} ulthread_state;

typedef enum ulthread_scheduling_algorithm
{
  ROUNDROBIN,
  PRIORITY,
  FCFS, // first-come-first serve
} ulthread_scheduling_algorithm;

typedef struct context
{
  uint64 ra;
  uint64 sp;
  uint64 s0;
  uint64 s1;
  uint64 s2;
  uint64 s3;
  uint64 s4;
  uint64 s5;
  uint64 s6;
  uint64 s7;
  uint64 s8;
  uint64 s9;
  uint64 s10;
  uint64 s11;
} context;

typedef struct ulthread
{
  uint64 ultid;
  uint64 start; // function starting address
  uint64 stack; // stack address
  uint64 priority;
  ulthread_state state;
  uint64 *args;
  struct context *context; // context save space
  uint64 creation_time;    // Time when thread was created (for FCFS)
} ulthread;

#endif