# ClankerOS Kernel Parameters

This document describes the kernel command line parameters supported by ClankerOS. Kernel parameters are passed via the bootloader (GRUB/Limine) using the `-append` option in QEMU or the `module` line in GRUB configuration.

## Usage

### QEMU
```bash
qemu-system-i386 -kernel kernel/clankeros.bin -append "earlycon root=ide:0.0.0"
```

### GRUB Configuration
```
menuentry "ClankerOS" {
    multiboot /boot/clankeros.bin earlycon root=ide:0.0.0
}
```

## Parameter Format

Parameters can be:
- **Boolean flags**: `earlycon`, `nographics`
- **Key-value pairs**: `root=ide:0.0.0`, `init=system/texthost.exe`

Multiple parameters are separated by spaces.

## Implemented Parameters

### `earlycon`
**Type**: Boolean flag
**Status**: ✅ Implemented (Session 4)
**Description**: Enable early console output via COM1 serial port.

Without this flag, serial output is disabled and only VGA output is available. Required for Claude Code testing and debugging.

**Example**:
```bash
qemu-system-i386 -kernel kernel/clankeros.bin -serial stdio -append "earlycon"
```

**Implementation**: [kernel/core/kcmdline.c](../kernel/core/kcmdline.c), [kernel/arch/i386/early_console.c](../kernel/arch/i386/early_console.c)

---

### `testpanic`
**Type**: Boolean flag
**Status**: ✅ Implemented (Session 5)
**Description**: Test the kernel panic system by triggering an intentional panic during boot.

Used for testing the KPanic macro and verifying panic message formatting works correctly.

**Example**:
```bash
qemu-system-i386 -kernel kernel/clankeros.bin -serial stdio -append "earlycon testpanic"
```

**Output**:
```
================================================================================
!!!                          KERNEL PANIC                                   !!!
================================================================================
Location: core/main.c:339
Message: Test panic - this is intentional (value: 42)

System halted. CPU in halt state.
================================================================================
```

**Implementation**: [kernel/core/main.c](../kernel/core/main.c)

---

### `testpagefault`
**Type**: Boolean flag
**Status**: ✅ Implemented (Session 5)
**Description**: Test the page fault handler and KPanicRegs by accessing an invalid memory address.

Used for testing the panic system with full register dump capabilities.

**Example**:
```bash
qemu-system-i386 -kernel kernel/clankeros.bin -serial stdio -append "earlycon testpagefault"
```

**Output**:
```
================================================================================
!!!                          KERNEL PANIC                                   !!!
================================================================================
Location: core/main.c:410
Message: Page Fault at 0xdeadbeef - Read from non-present page

CPU Register Dump:
  EIP: 0x001011d1  CS:  0x0008  EFLAGS: 0x00010246
  EAX: 0x0000002c  EBX: 0x00106008  ECX: 0x001028f8  EDX: 0x00000000
  ESP: 0x0010af94  EBP: 0x0010aff0  ESI: 0x0010448e  EDI: 0x00106010
  DS:  0x0010  SS:  0x4d58
  INT: 14  ERR: 0x00000000

System halted. CPU in halt state.
================================================================================
```

**Implementation**: [kernel/core/main.c](../kernel/core/main.c)

---

## Planned Parameters (Not Yet Implemented)

### `root=<block_device>`
**Type**: Key-value
**Status**: ⏳ Planned
**Description**: Specify the root filesystem block device.

Uses the block device namespace format: `block:<interface>:<address>`

**Examples**:
```bash
# IDE primary master
root=ide:0.0.0

# First floppy disk
root=floppy:a

# USB mass storage device
root=usb:001.008

# First NVMe device
root=nvme:0
```

**Default**: If not specified, kernel will attempt to auto-detect or use a compiled-in default.

**Related**: See [IPC_ARCHITECTURE.md](IPC_ARCHITECTURE.md) for block device namespace design.

---

### `nographics`
**Type**: Boolean flag
**Status**: ⏳ Planned
**Description**: Disable graphics output and remain in text mode.

Prevents the kernel from initializing any graphics drivers or switching to a graphical framebuffer mode. The system will stay in VGA text mode (80x25 characters).

Useful for:
- Headless servers
- Serial console-only systems
- Debugging graphics issues

**Example**:
```bash
qemu-system-i386 -kernel kernel/clankeros.bin -append "earlycon nographics"
```

**Implementation Note**: When graphics are implemented, this flag should be checked before initializing the graphics subsystem.

---

### `init=<program_path>`
**Type**: Key-value
**Status**: ⏳ Planned
**Description**: Specify an alternate initial process instead of the default init.

The path is relative to the root filesystem. After kernel initialization completes, this program will be loaded and executed as PID 1.

**Examples**:
```bash
# Use custom text-based host
init=system/texthost.exe

# Use graphical desktop environment
init=system/desktop.exe

# Boot into recovery shell
init=system/shell.exe
```

**Default**: If not specified, the kernel will load the default init process (typically `system/init.exe`).

**Implementation Note**: Will be used during process loading after filesystem support is implemented.

---

## Adding New Parameters

To add a new kernel parameter:

1. **Update the parser** in [kernel/core/kcmdline.c](../kernel/core/kcmdline.c)
   - For boolean flags: Use `KCmdLineHasFlag()`
   - For key-value pairs: Use `KCmdLineGetValue()`

2. **Check the parameter** in [kernel/core/main.c](../kernel/core/main.c) (or appropriate subsystem)
   ```c
   if (KCmdLineHasFlag("myflag")) {
       // Handle flag
   }

   const char* value = KCmdLineGetValue("mykey");
   if (value) {
       // Use value
   }
   ```

3. **Document it** in this file with:
   - Parameter name and type
   - Status (Implemented/Planned)
   - Description and usage examples
   - Implementation notes

## API Reference

See [kernel/include/kcmdline.h](../kernel/include/kcmdline.h) for the complete API:

```c
// Initialize command line parser (call early in KMain)
void KCmdLineInitialize(multiboot_info_t* mbootInfo);

// Check for boolean flag
bool KCmdLineHasFlag(const char* flag);

// Get value for key=value parameter
const char* KCmdLineGetValue(const char* key);
```

## Testing

Test kernel parameters in QEMU:

```bash
# Test single parameter
qemu-system-i386 -kernel kernel/clankeros.bin -serial stdio -append "earlycon"

# Test multiple parameters
qemu-system-i386 -kernel kernel/clankeros.bin -serial stdio \
    -append "earlycon nographics root=ide:0.0.0"

# Test key-value parameters
qemu-system-i386 -kernel kernel/clankeros.bin -serial stdio \
    -append "earlycon root=floppy:a init=system/texthost.exe"
```
