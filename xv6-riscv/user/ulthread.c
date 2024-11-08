/* CSE 536: User-Level Threading Library */
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "user/ulthread.h"

/* Standard definitions */
#include <stdbool.h>
#include <stddef.h>

static ulthread ulthred_table[MAXULTHREADS];

/* Get thread ID */
int get_current_tid(void)
{
    return 0;
}

struct ulthread *scheduler_thread = NULL;
static ulthread *current_thread = NULL;

static ulthread_scheduling_algorithm scheduling_algorithm = ROUNDROBIN;
static uint64 global_thread_counter = 0;
/* Thread initialization */
void ulthread_init(int schedalgo)
{
    memset(ulthred_table, 0, sizeof(ulthred_table));
    for (int i = 0; i < MAXULTHREADS; i++)
    {
        ulthred_table[i].state = FREE;
    }
    scheduling_algorithm = schedalgo;

    // Create the scheduler thread using the current execution context
    current_thread = &ulthred_table[0];
    current_thread->ultid = 0;
    current_thread->state = RUNNABLE;
    current_thread->priority = 0;
    current_thread->creation_time = global_thread_counter++;

    // Set the current thread to the scheduler thread
    current_thread = scheduler_thread;
}

/* Thread creation */
bool ulthread_create(uint64 start, uint64 stack, uint64 args[], int priority)
{
    ulthread *ult = NULL;
    for (int i = 0; i < MAXULTHREADS; i++)
    {
        if (ulthred_table[i].ultid == 0)
        {
            ult = &ulthred_table[i];
            ult->ultid = i;
            break;
        }
    }

    if (ult == NULL)
    {
        printf("No available thread slots\n");
        return false;
    }

    ult->state = RUNNABLE;
    ult->priority = priority;
    ult->start = start;
    ult->stack = stack;
    ult->args = args;

    // Initialize the context
    memset(&(ult->context), 0, sizeof(struct context));

    printf("[*] ultcreate(tid: %d, ra: %p, sp: %p)\n", ult->ultid, start, stack);

    return true;
}

/* Thread scheduler */
void ulthread_schedule(void)
{
    ulthread *prev_thread = current_thread;
    ulthread *next_thread = NULL;

    if (scheduling_algorithm == ROUNDROBIN)
    {
        // Round-robin scheduling
        int next_tid = (current_thread->ultid + 1) % MAXULTHREADS;
        for (int i = 0; i < MAXULTHREADS; i++)
        {
            int index = (next_tid + i) % MAXULTHREADS;
            if (ulthred_table[index].state == RUNNABLE)
            {
                next_thread = &ulthred_table[index];
                break;
            }
            else if (ulthred_table[index].state == YIELD)
            {
                // Skip YIELD threads this time; set them back to RUNNABLE
                ulthred_table[index].state = RUNNABLE;
            }
        }
    }
    else if (scheduling_algorithm == PRIORITY)
    {
        // Priority scheduling
        uint64 highest_priority = 0xFFFFFFFFFFFFFFFF;
        for (int i = 0; i < MAXULTHREADS; i++)
        {
            if (ulthred_table[i].state == RUNNABLE)
            {
                if (ulthred_table[i].priority < highest_priority)
                {
                    highest_priority = ulthred_table[i].priority;
                    next_thread = &ulthred_table[i];
                }
            }
            else if (ulthred_table[i].state == YIELD)
            {
                // Skip YIELD threads this time; set them back to RUNNABLE
                ulthred_table[i].state = RUNNABLE;
            }
        }
    }
    else if (scheduling_algorithm == FCFS)
    {
        // First-Come-First-Serve scheduling
        uint64 earliest_time = 0xFFFFFFFFFFFFFFFF;
        for (int i = 0; i < MAXULTHREADS; i++)
        {
            if (ulthred_table[i].state == RUNNABLE)
            {
                if (ulthred_table[i].creation_time < earliest_time)
                {
                    earliest_time = ulthred_table[i].creation_time;
                    next_thread = &ulthred_table[i];
                }
            }
            else if (ulthred_table[i].state == YIELD)
            {
                // Skip YIELD threads this time; set them back to RUNNABLE
                ulthred_table[i].state = RUNNABLE;
            }
        }
    }

    /* Add this statement to denote which thread-id is being scheduled next */
    printf("[*] ultschedule (next tid: %d)\n", next_thread->ultid);
    current_thread = next_thread;
    // Switch between thread contexts
    ulthread_context_switch(&prev_thread->context, &next_thread->context);
}

/* Yield CPU time to some other thread. */
void ulthread_yield(void)
{
    current_thread->state = YIELD;
    printf("[*] ultyield(tid: %d)\n", current_thread->ultid);
    ulthread_schedule();
}

/* Destroy thread */
void ulthread_destroy(void) {}
