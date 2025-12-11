#!/usr/bin/env python3
"""
Disk Image Creation Script for ClankerOS
Creates bootable disk images for ClankerOS.

Compatible with MicroPython.

TODO: Implement once we have a filesystem worth testing.
For now, use QEMU's built-in multiboot support:
  qemu-system-i386 -kernel kernel.bin
"""

import sys


def main():
    print("mkdisk.py: Not yet implemented")
    print("For testing, use QEMU's multiboot support:")
    print("  qemu-system-i386 -kernel kernel.bin")
    return 0


if __name__ == '__main__':
    sys.exit(main())
