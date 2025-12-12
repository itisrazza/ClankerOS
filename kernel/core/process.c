/* process.c - Process Management Implementation */

#include "process.h"
#include "kheap.h"
#include "pmm.h"
#include "paging.h"
#include "isr.h"
#include "panic.h"
#include "clc/printf.h"
#include "econ_writer.h"
#include "vid_writer.h"
#include <stddef.h>

/* Memory layout constants */
#define KERNEL_STACK_SIZE 8192  // 8 KB kernel stack per process

/* Process management state */
static Process* currentProcess = NULL;
static Process* readyQueueHead = NULL;
static Process* readyQueueTail = NULL;
static uint32_t nextPid = 1;
static bool schedulerEnabled = false;

/* Forward declarations */
static void processEntry(void);
static void enqueueProcess(Process* process);
static Process* dequeueProcess(void);
static void saveContext(Process* process, registers_t* regs);
static void restoreContext(Process* process, registers_t* regs);

/*
 * ProcessInitialize - Initialize the process management system
 */
void ProcessInitialize(void)
{
    ClcWriter* serial = EConGetWriter();
    ClcPrintfWriter(serial, "Initializing process management...\n");

    // Create the initial kernel process (represents current execution context)
    currentProcess = (Process*)KAllocateMemory(sizeof(Process));
    if (!currentProcess) {
        ClcPrintfWriter(serial, "Failed to allocate initial process!\n");
        return;
    }

    // Initialize fields
    currentProcess->pid = 0;
    const char* initName = "idle";
    for (int i = 0; i < 32; i++) currentProcess->name[i] = 0;
    for (int i = 0; initName[i] && i < 31; i++) {
        currentProcess->name[i] = initName[i];
    }

    currentProcess->state = PROCESS_STATE_RUNNING;
    currentProcess->mode = PROCESS_MODE_KERNEL;
    currentProcess->pageDirectory = PagingGetCurrentDirectory();
    currentProcess->kernelStack = 0;  // Using boot stack
    currentProcess->userStack = 0;
    currentProcess->timeslice = 10;
    currentProcess->priority = 0;
    currentProcess->next = NULL;

    // Initialize ready queue
    readyQueueHead = NULL;
    readyQueueTail = NULL;

    ClcPrintfWriter(serial, "Process management initialized (PID 0: idle)\n");
}

/*
 * ProcessCreate - Create a new process
 */
Process* ProcessCreate(const char* name, void (*entryPoint)(void), ProcessMode mode)
{
    ClcWriter* serial = EConGetWriter();

    // Allocate process structure
    Process* process = (Process*)KAllocateMemory(sizeof(Process));
    if (!process) {
        ClcPrintfWriter(serial, "Failed to allocate process structure\n");
        return NULL;
    }

    // Initialize basic fields
    process->pid = nextPid++;
    for (int i = 0; i < 32; i++) process->name[i] = 0;
    for (int i = 0; name && name[i] && i < 31; i++) {
        process->name[i] = name[i];
    }
    process->state = PROCESS_STATE_READY;
    process->mode = mode;
    process->timeslice = 10;
    process->priority = 0;
    process->next = NULL;

    // Allocate kernel stack
    process->kernelStack = (uintptr_t)KAllocateMemory(KERNEL_STACK_SIZE);
    if (!process->kernelStack) {
        ClcPrintfWriter(serial, "Failed to allocate kernel stack\n");
        KFreeMemory(process);
        return NULL;
    }

    // For now, all processes use the kernel page directory
    process->pageDirectory = PagingGetCurrentDirectory();
    process->userStack = 0;

    // Set up initial context
    // We'll manually create a context that looks like it was interrupted
    // Stack grows downward
    uintptr_t stackTop = process->kernelStack + KERNEL_STACK_SIZE;

    // Set up the stack to contain:
    // 1. Entry point address (for wrapper to call)
    // 2. Fake interrupt frame

    uint32_t* stack = (uint32_t*)stackTop;

    // Push entry point (will be used by processEntry wrapper)
    *(--stack) = (uint32_t)entryPoint;

    // Now set up fake interrupt frame (as if process was interrupted)
    // This matches what the isr_common_stub expects when returning

    if (mode == PROCESS_MODE_USER) {
        // User mode (not fully implemented yet)
        *(--stack) = 0x23;      // SS (user data segment with RPL=3)
        *(--stack) = 0xC0000000;  // User ESP
    }

    *(--stack) = 0x202;         // EFLAGS (IF=1, reserved bit 1=1)
    *(--stack) = (mode == PROCESS_MODE_KERNEL) ? 0x08 : 0x1B;  // CS
    *(--stack) = (uint32_t)processEntry;  // EIP (entry wrapper)

    *(--stack) = 0;             // Error code (not used)
    *(--stack) = 0;             // Interrupt number (not used)

    *(--stack) = (mode == PROCESS_MODE_KERNEL) ? 0x10 : 0x23;  // DS

    // PUSHA register values (all zeroed for new process)
    *(--stack) = 0;             // EDI
    *(--stack) = 0;             // ESI
    *(--stack) = 0;             // EBP
    *(--stack) = 0;             // ESP (ignored by POPA)
    *(--stack) = 0;             // EBX
    *(--stack) = 0;             // EDX
    *(--stack) = 0;             // ECX
    *(--stack) = 0;             // EAX

    // Now set the process's ESP to point to this prepared stack
    process->context.esp = (uint32_t)stack;
    process->context.eip = (uint32_t)processEntry;
    process->context.cs = (mode == PROCESS_MODE_KERNEL) ? 0x08 : 0x1B;
    process->context.eflags = 0x202;
    process->context.ss = (mode == PROCESS_MODE_KERNEL) ? 0x10 : 0x23;
    process->context.ds = (mode == PROCESS_MODE_KERNEL) ? 0x10 : 0x23;
    process->context.es = process->context.ds;
    process->context.fs = process->context.ds;
    process->context.gs = process->context.ds;

    // Other registers
    process->context.eax = process->context.ebx = process->context.ecx = 0;
    process->context.edx = process->context.esi = process->context.edi = 0;
    process->context.ebp = 0;
    process->context.useresp = (mode == PROCESS_MODE_USER) ? 0xC0000000 : 0;

    // Add to ready queue
    enqueueProcess(process);

    ClcPrintfWriter(serial, "Created process PID %u: %s (%s mode)\n",
                    process->pid, process->name,
                    mode == PROCESS_MODE_KERNEL ? "kernel" : "user");

    return process;
}

/*
 * ProcessDestroy - Destroy a process
 */
void ProcessDestroy(Process* process)
{
    if (!process) return;

    // Free kernel stack
    if (process->kernelStack) {
        KFreeMemory((void*)process->kernelStack);
    }

    // Free process structure
    KFreeMemory(process);
}

/*
 * ProcessGetCurrent - Get currently running process
 */
Process* ProcessGetCurrent(void)
{
    return currentProcess;
}

/*
 * ProcessYield - Voluntarily yield CPU
 */
void ProcessYield(void)
{
    if (!schedulerEnabled) return;

    // Software interrupt to trigger scheduler
    __asm__ volatile ("int $0x81");
}

/*
 * ProcessSchedule - Schedule next process (called from interrupt)
 */
void ProcessSchedule(registers_t* regs)
{
    KAssert(regs != NULL, "ProcessSchedule called with NULL registers");

    if (!schedulerEnabled || !currentProcess) {
        return;
    }

    // Save current process state
    if (currentProcess->state == PROCESS_STATE_RUNNING) {
        saveContext(currentProcess, regs);
        currentProcess->state = PROCESS_STATE_READY;
        currentProcess->timeslice--;

        // Re-queue if still has time or reset timeslice
        if (currentProcess->timeslice == 0) {
            currentProcess->timeslice = 10;
        }
        enqueueProcess(currentProcess);
    } else if (currentProcess->state == PROCESS_STATE_TERMINATED) {
        // Process is terminated, don't save context or re-queue
        // It will be cleaned up later (for now just leave it)
    }

    // Get next process from ready queue
    Process* nextProcess = dequeueProcess();

    if (!nextProcess) {
        // No process to run, keep current
        currentProcess->state = PROCESS_STATE_RUNNING;
        return;
    }

    // Switch to next process
    Process* oldProcess = currentProcess;
    currentProcess = nextProcess;
    currentProcess->state = PROCESS_STATE_RUNNING;
    currentProcess->timeslice = 10;

    // Switch page directory if different
    if (oldProcess->pageDirectory != currentProcess->pageDirectory) {
        PagingSwitchDirectory((uintptr_t)currentProcess->pageDirectory);
    }

    // Restore new process context into interrupt frame
    restoreContext(currentProcess, regs);
}

/*
 * ProcessBlock - Block current process
 */
void ProcessBlock(void)
{
    if (!currentProcess || !schedulerEnabled) return;

    currentProcess->state = PROCESS_STATE_BLOCKED;
    ProcessYield();  // Trigger scheduler
}

/*
 * ProcessUnblock - Unblock a process
 */
void ProcessUnblock(Process* process)
{
    if (!process) return;

    if (process->state == PROCESS_STATE_BLOCKED) {
        process->state = PROCESS_STATE_READY;
        enqueueProcess(process);
    }
}

/*
 * ProcessExit - Terminate current process
 */
void ProcessExit(void)
{
    if (!currentProcess) return;

    ClcWriter* serial = EConGetWriter();
    ClcPrintfWriter(serial, "Process %u (%s) exiting\n",
                    currentProcess->pid, currentProcess->name);

    currentProcess->state = PROCESS_STATE_TERMINATED;

    // Just wait for the next timer interrupt to switch us out
    // The scheduler will see we're terminated and won't reschedule us
    while (1) {
        __asm__ volatile ("hlt");
    }
}

/*
 * ProcessEnableScheduler - Enable the scheduler
 */
void ProcessEnableScheduler(void)
{
    schedulerEnabled = true;
}

/*
 * processEntry - Entry wrapper for all processes
 *
 * This function is the first thing that runs when a process starts.
 * The actual entry point is passed on the stack.
 */
static void processEntry(void)
{
    // Enable interrupts
    __asm__ volatile ("sti");

    // Get entry point from stack (placed there during process creation)
    void (*entryPoint)(void);
    __asm__ volatile ("mov 4(%%ebp), %0" : "=r"(entryPoint));

    // Call the actual entry point
    if (entryPoint) {
        entryPoint();
    }

    // If the entry point returns, exit the process
    ProcessExit();
}

/*
 * saveContext - Save CPU context from interrupt frame to process
 */
static void saveContext(Process* process, registers_t* regs)
{
    if (!process || !regs) return;

    // Save general purpose registers (from pusha)
    process->context.edi = regs->edi;
    process->context.esi = regs->esi;
    process->context.ebp = regs->ebp;
    process->context.esp = regs->esp;
    process->context.ebx = regs->ebx;
    process->context.edx = regs->edx;
    process->context.ecx = regs->ecx;
    process->context.eax = regs->eax;

    // Save segment registers
    process->context.ds = regs->ds;
    process->context.es = process->context.fs = process->context.gs = regs->ds;

    // Save instruction pointer and flags (from CPU)
    process->context.eip = regs->eip;
    process->context.cs = regs->cs;
    process->context.eflags = regs->eflags;

    // Save user ESP and SS if available
    process->context.useresp = regs->useresp;
    process->context.ss = regs->ss;
}

/*
 * restoreContext - Restore CPU context from process to interrupt frame
 */
static void restoreContext(Process* process, registers_t* regs)
{
    if (!process || !regs) return;

    // Restore general purpose registers (will be restored by popa)
    regs->edi = process->context.edi;
    regs->esi = process->context.esi;
    regs->ebp = process->context.ebp;
    regs->esp = process->context.esp;
    regs->ebx = process->context.ebx;
    regs->edx = process->context.edx;
    regs->ecx = process->context.ecx;
    regs->eax = process->context.eax;

    // Restore segment registers
    regs->ds = process->context.ds;

    // Restore instruction pointer and flags (will be restored by iret)
    regs->eip = process->context.eip;
    regs->cs = process->context.cs;
    regs->eflags = process->context.eflags;

    // Restore user ESP and SS
    regs->useresp = process->context.useresp;
    regs->ss = process->context.ss;
}

/*
 * enqueueProcess - Add process to ready queue
 */
static void enqueueProcess(Process* process)
{
    if (!process) return;

    process->next = NULL;

    if (!readyQueueHead) {
        readyQueueHead = process;
        readyQueueTail = process;
    } else {
        readyQueueTail->next = process;
        readyQueueTail = process;
    }
}

/*
 * dequeueProcess - Remove and return next process from ready queue
 */
static Process* dequeueProcess(void)
{
    if (!readyQueueHead) {
        return NULL;
    }

    Process* process = readyQueueHead;
    readyQueueHead = process->next;

    if (!readyQueueHead) {
        readyQueueTail = NULL;
    }

    process->next = NULL;
    return process;
}
