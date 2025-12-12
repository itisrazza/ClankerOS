/* process.h - Process Management */
#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <stdbool.h>
#include "paging.h"
#include "isr.h"

/* Process states */
typedef enum {
    PROCESS_STATE_READY,      // Ready to run
    PROCESS_STATE_RUNNING,    // Currently running
    PROCESS_STATE_BLOCKED,    // Blocked waiting for IPC or I/O
    PROCESS_STATE_TERMINATED  // Process has terminated
} ProcessState;

/* Process privilege levels */
typedef enum {
    PROCESS_MODE_KERNEL,      // Ring 0 - kernel mode
    PROCESS_MODE_USER         // Ring 3 - user mode
} ProcessMode;

/* CPU context for context switching */
typedef struct {
    // General purpose registers (saved by pusha)
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;

    // Segment registers
    uint32_t ds, es, fs, gs;

    // Instruction pointer and flags (saved by CPU on interrupt)
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;

    // Stack pointer and segment (for user mode)
    uint32_t useresp;
    uint32_t ss;
} __attribute__((packed)) CpuContext;

/* Process Control Block (PCB) */
typedef struct Process {
    uint32_t pid;                    // Process ID
    char name[32];                   // Process name
    ProcessState state;              // Current state
    ProcessMode mode;                // Kernel or user mode

    CpuContext context;              // CPU context for context switching
    uintptr_t kernelStack;           // Kernel stack pointer (virtual address)
    uintptr_t userStack;             // User stack pointer (for user mode processes)

    PageDirectory* pageDirectory;    // Page directory (physical address)

    // Scheduling
    uint32_t timeslice;              // Time slices remaining
    uint32_t priority;               // Priority level (unused for now)

    // Linked list for process queue
    struct Process* next;
} Process;

/*
 * ProcessInitialize - Initialize the process management system
 *
 * Sets up the initial kernel process and prepares the scheduler.
 */
void ProcessInitialize(void);

/*
 * ProcessCreate - Create a new process
 *
 * @name: Process name
 * @entryPoint: Entry point function
 * @mode: Kernel or user mode
 * @return: Pointer to created process, or NULL on failure
 */
Process* ProcessCreate(const char* name, void (*entryPoint)(void), ProcessMode mode);

/*
 * ProcessDestroy - Destroy a process
 *
 * @process: Process to destroy
 */
void ProcessDestroy(Process* process);

/*
 * ProcessGetCurrent - Get currently running process
 *
 * @return: Pointer to current process
 */
Process* ProcessGetCurrent(void);

/*
 * ProcessYield - Voluntarily yield CPU to another process
 *
 * Triggers a context switch to the next ready process.
 */
void ProcessYield(void);

/*
 * ProcessSchedule - Schedule next process to run
 *
 * Called by timer interrupt or when process yields/blocks.
 * Performs context switch to next ready process.
 *
 * @regs: Pointer to interrupt register state
 */
void ProcessSchedule(registers_t* regs);

/*
 * ProcessBlock - Block current process
 *
 * Marks the current process as blocked and schedules another process.
 */
void ProcessBlock(void);

/*
 * ProcessUnblock - Unblock a process
 *
 * @process: Process to unblock
 */
void ProcessUnblock(Process* process);

/*
 * ProcessExit - Terminate current process
 *
 * Marks current process as terminated and schedules another process.
 */
void ProcessExit(void);

/*
 * ProcessEnableScheduler - Enable the process scheduler
 *
 * Enables preemptive multitasking. Should be called after creating
 * initial processes and setting up timer interrupts.
 */
void ProcessEnableScheduler(void);

#endif /* PROCESS_H */
