# ClankerOS IPC Architecture

## Overview

ClankerOS uses a URI-based namespace system for interprocess communication, departing from traditional POSIX-style syscalls. Programs register namespaces and communicate via structured messages over synchronous IPC channels.

## Core Concepts

### URI-based Namespaces

Programs can register and serve namespaces using URI-like schemes:

```
fs:root:program/hello.exe    - Filesystem access
proc:123:memory              - Process information
windows:desktop:main         - Window management
block:ide:0.0.0              - Block device access (IDE primary master)
posix:vfs:/bin/hello         - POSIX compatibility layer
```

**Aliases** provide convenient shortcuts:
- `home:/` → user's home directory
- `root:/` → root filesystem

### Block Device Namespace Design

Block devices use **interface-based sub-namespaces** followed by addressing:

```
block:<interface>:<address>
```

**Examples:**
- `block:floppy:a` - First floppy disk (A: under DOS)
- `block:floppy:b` - Second floppy disk (B: under DOS)
- `block:ide:0.0.0` - IDE primary master (first controller, channel 0, device 0)
- `block:ide:0.0.1` - IDE primary slave
- `block:ide:0.1.0` - IDE secondary master
- `block:ide:0.1.1` - IDE secondary slave
- `block:usb:001.008` - USB mass storage (bus 1, device 8)
- `block:nvme:0` - First NVMe device
- `block:nvme:1` - Second NVMe device
- `block:scsi:0.1.0` - SCSI controller 0, target 1, LUN 0

**Rationale:**
- Interface-specific addressing reflects hardware topology
- No Linux-style generic naming (`sda`, `mmcblk`) that obscures device type
- Clear mapping to physical hardware for debugging
- Each interface can define its own addressing scheme appropriate to the hardware

### Kernel-mode vs User-mode Programs

**Kernel-mode programs:**
- Scheduled like regular processes
- Direct access to hardware and interrupts
- Can receive hardware interrupts
- Access to kernel memory management and scheduling primitives
- Function table for page table switching
- Act as servers for device drivers and system services

**User-mode programs:**
- Traditional userspace with restricted privileges
- Communicate with kernel-mode servers via IPC
- Cannot directly access hardware

**Both** use the same IPC mechanism for communication.

### POSIX Compatibility Layer

POSIX support is implemented as a **kernel-mode program**:
- Receives traditional UNIX syscalls from client processes
- Translates to native ClankerOS namespace operations
- Provides POSIX environment without baking it into the kernel
- Example: `open("/bin/hello")` → IPC to `posix:vfs:/bin/hello`

## IPC Design Decisions

### Message Passing Model

**Synchronous by default:**
- Caller blocks until server responds
- Simple mental model, easier to debug
- Servers can provide async patterns as higher-level abstractions:
  - Return job IDs for long-running operations
  - Schedule process interrupts (signals) on completion

Example:
```c
// Synchronous read - blocks until data available
Message* response = IpcSend("fs:root:file.txt", MSG_READ, ...);

// Async pattern - server returns immediately with job ID
Message* jobMsg = IpcSend("fs:root:largefile.dat", MSG_READ_ASYNC, ...);
uint32_t jobId = jobMsg->data.jobId;
// ... later, receive signal when job completes
```

### Namespace Registration

**Dynamic registration at runtime:**
- Programs register namespaces when they start
- Can unregister when shutting down
- No static boot-time namespace allocation

**Hierarchical namespaces:**
- `fs:root:` and `fs:temp:` can be served by different programs
- Namespace registry does longest-prefix matching

Example:
```c
// Filesystem driver registers namespace
NamespaceRegister("fs:root:", myServerHandler);

// Later, unregister on shutdown
NamespaceUnregister("fs:root:");
```

### Security and Access Control

**No access control for now** (hobby project scope limitation)
- Any process can send to any namespace
- Can be added later with capability tokens/handles

### Message Format

**Structured messages with typed fields:**
- Standardized format for debuggability
- Implemented as buffers with length under the hood
- Type field indicates message purpose

Preliminary structure:
```c
typedef enum {
    MSG_READ,
    MSG_WRITE,
    MSG_OPEN,
    MSG_CLOSE,
    MSG_QUERY,
    MSG_RESPONSE,
    MSG_ERROR,
    // ... extensible
} MessageType;

typedef struct {
    MessageType type;
    uint32_t length;        // Total message length
    uint32_t senderId;      // Sender process ID
    uint32_t requestId;     // For matching responses
    uint8_t data[];         // Variable-length payload
} Message;
```

### Kernel-mode Program API

**Direct access:**
- Memory management functions (kmalloc, kfree, page allocation)
- Scheduling primitives (yield, sleep, wake)
- Hardware I/O (inb, outb, MMIO)
- Interrupt registration

**Function table for page switching:**
```c
typedef struct {
    void* (*MemAllocatePage)(void);
    void (*MemFreePage)(void* page);
    void (*MemSwitchPageDirectory)(void* pageDir);
    void (*SchedYield)(void);
    void (*IrqRegister)(uint8_t irq, void (*handler)(void));
    // ... other kernel services
} KernelApi;
```

## Implementation Phases

### Phase 1: Memory Management (Current Milestone)

**Goal:** Basic memory infrastructure for processes

1. **Physical memory allocator**
   - Bitmap or stack-based allocator
   - Parse multiboot memory map for available RAM
   - Track free/used pages (4KB)

2. **Paging (minimal)**
   - Set up identity mapping for kernel
   - Enable paging (CR0.PG bit)
   - Page fault handler (ISR 14)

3. **Kernel heap**
   - Simple kmalloc/kfree for dynamic allocation
   - Bump allocator or simple block allocator

**Deliverables:**
- Physical page allocator working
- Paging enabled with identity mapping
- kmalloc/kfree functional

### Phase 2: Process Management

**Goal:** Schedulable processes with isolation

1. **Process structure (PCB)**
   - Process ID, state (running/ready/blocked)
   - Page directory pointer
   - Kernel-mode vs user-mode flag
   - CPU context (registers, stack pointers)
   - Message queue for IPC

2. **Context switching**
   - Save/restore CPU state (all registers, ESP, EIP)
   - Switch page directories (CR3)
   - Handle kernel-mode vs user-mode stack switching

3. **Basic scheduler**
   - Round-robin scheduling
   - Ready queue for both kernel-mode and user-mode programs
   - Timer interrupt (PIT) for preemption

4. **User mode transition**
   - Switch to ring 3 from ring 0
   - Syscall interface (int 0x80 or sysenter)
   - Return to user mode (iret)

**Deliverables:**
- Multiple processes can run
- Scheduler switches between them
- Kernel-mode and user-mode processes both work

### Phase 3: IPC & Namespace System

**Goal:** Working message passing between processes

1. **Message structure**
   - Define Message struct with typed fields
   - Message allocation/deallocation
   - Message serialization (if needed)

2. **Namespace registry**
   - Hash table or tree for namespace lookup
   - Register/unregister/lookup functions
   - Longest-prefix matching for hierarchical namespaces

3. **Message passing**
   - `IpcSend(namespace, message)` - synchronous send
   - `IpcReceive()` - block until message arrives
   - Message queuing in process PCB
   - Scheduler integration (block sender, wake receiver)

4. **Kernel-mode program support**
   - Define KernelApi function table
   - Provide to kernel-mode programs at startup
   - Page table switching helpers

**Deliverables:**
- Processes can register namespaces
- Processes can send/receive messages
- Synchronous blocking IPC works

### Phase 4: Basic Servers

**Goal:** Demonstrate IPC with real services

1. **Process namespace (`proc:`)**
   - Kernel-mode server
   - Query running processes: `proc:list:`
   - Query process info: `proc:123:memory`, `proc:123:state`

2. **Simple filesystem (`fs:`)**
   - RAM-based filesystem or initrd support
   - Basic operations: read, write, open, close
   - Example: `fs:root:file.txt`

3. **POSIX compatibility layer (stretch goal)**
   - Kernel-mode program
   - Intercept int 0x80 syscalls
   - Translate to IPC messages
   - Provide VFS abstraction: `posix:vfs:/bin/hello`

**Deliverables:**
- At least 2 working namespace servers
- Demonstrate IPC between programs
- Basic file operations work

## Design Rationale

### Why URI-style Namespaces?

- **Discoverability:** Programs can explore namespaces like URLs
- **Flexibility:** Easy to add new services without kernel changes
- **Hierarchical:** Natural tree structure for resources
- **Familiar:** Developers understand URI semantics

### Why Kernel-mode Programs?

- **Performance:** Device drivers need fast hardware access
- **Flexibility:** Drivers can be scheduled and isolated like processes
- **Microkernel-inspired:** Move functionality out of core kernel
- **Simplicity:** Same IPC mechanism for all communication

### Why Synchronous IPC?

- **Simplicity:** Easier to reason about and debug
- **Sufficient:** Async can be built on top (job IDs, signals)
- **Deterministic:** Clear caller/callee relationship

## Future Considerations

**Not in initial scope:**
- Access control / capability system
- Distributed IPC across machines
- Zero-copy message passing
- Shared memory IPC
- Async/await syntax for IPC

These can be added later as the OS matures.

## Examples

### Filesystem Server (Kernel-mode)

```c
void FsServerMain(KernelApi* kapi)
{
    NamespaceRegister("fs:root:", FsRootHandler);

    while (1) {
        Message* msg = IpcReceive();

        switch (msg->type) {
            case MSG_READ:
                FsHandleRead(msg);
                break;
            case MSG_WRITE:
                FsHandleWrite(msg);
                break;
        }
    }
}

void FsHandleRead(Message* msg)
{
    // Read file data
    char* path = (char*)msg->data;
    char* fileData = FsReadFile(path);

    // Send response
    Message* response = CreateMessage(MSG_RESPONSE, strlen(fileData));
    memcpy(response->data, fileData, strlen(fileData));
    IpcReply(msg->senderId, response);
}
```

### Client Reading a File (User-mode)

```c
void UserProgram(void)
{
    Message* msg = CreateMessage(MSG_READ, 20);
    strcpy(msg->data, "fs:root:hello.txt");

    // Blocks until filesystem server responds
    Message* response = IpcSend("fs:root:hello.txt", msg);

    if (response->type == MSG_RESPONSE) {
        printf("File contents: %s\n", response->data);
    }

    FreeMessage(response);
}
```

### POSIX Compatibility Layer

```c
// POSIX server intercepts syscalls
void PosixSyscallHandler(uint32_t syscallNum, ...)
{
    switch (syscallNum) {
        case SYS_OPEN:
            char* path = va_arg(args, char*);
            // Translate to IPC
            Message* msg = CreateMessage(MSG_OPEN, strlen(path));
            strcpy(msg->data, path);
            Message* response = IpcSend("fs:root:", msg);
            return response->data.fd;

        case SYS_READ:
            // ... similar translation
    }
}
```

## References

- Multiboot specification: https://www.gnu.org/software/grub/manual/multiboot/
- Intel x86 manual (paging, privilege levels)
- Microkernel design principles (Tanenbaum)
- Plan 9 namespace concepts (inspiration)
