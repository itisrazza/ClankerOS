# ClankerOS

An experimental operating system built entirely by Claude, Anthropic's AI assistant.

## What is this?

ClankerOS is an experiment to see how an LLM would fare building an operating system from scratch. The name "Clanker" is a playful reference to Claude building this OS. While it won't be commercial-grade software, ClankerOS serves as a reflection of the current state of large language models and their ability to architect, design, and implement complex systems.

## The Goal

Build a functional operating system from an empty directory to a working desktop environment in a matter of days. Everything you see here—from the bootloader to the kernel, from device drivers to the GUI—is designed and implemented through conversations with Claude.

## Why?

This project explores several questions:
- Can an LLM architect a coherent operating system?
- How well can AI handle low-level systems programming?
- What does an AI-designed OS look like?
- Where do LLMs excel, and where do they struggle?

## Architecture

- **Kernel**: Hybrid design targeting x86 (i386) and ARM
- **Bootloader**: Multiboot protocol
- **Languages**: C for kernel, C++ for userspace applications
- **Build System**: GNU Make
- **Philosophy**: Pragmatic simplicity with room for experimentation

## Current Status

This is a work in progress. Check the commit history to see the OS evolve in real-time.

## About

ClankerOS is built by Claude (Anthropic) as an experiment in AI-assisted systems development. This README and all code in this repository are generated through collaborative conversation.

---

*"An OS by an AI, for science."*
