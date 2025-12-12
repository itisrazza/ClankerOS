# ClankerOS Root Makefile

# Include common configuration
include Makefile.common

.PHONY: all kernel programs libraries clean run run-serial debug toolchain website docs serve-website help

# Build all components
all: kernel

# Build kernel
kernel:
	$(MAKE) -C kernel

# Build userspace programs
programs:
	$(MAKE) -C programs

# Build libraries
libraries:
	$(MAKE) -C libraries

# Clean all build artifacts
clean:
	$(MAKE) -C kernel clean
	$(MAKE) -C programs clean
	$(MAKE) -C libraries clean

# Run in QEMU
run: kernel
	qemu-system-i386 -kernel kernel/clankeros.bin

# Run in QEMU with serial output
run-serial: kernel
	qemu-system-i386 -kernel kernel/clankeros.bin -serial stdio

# Run in QEMU with debugging
debug: kernel
	qemu-system-i386 -kernel kernel/clankeros.bin -s -S

# Build toolchain
toolchain:
	bash scripts/build-toolchain.sh

# Documentation and website targets
website: docs
	@echo "Generating website..."
	python3 scripts/generate_website.py
	python3 scripts/convert_markdown.py
	@echo "Done! Website generated in website/"

docs:
	@echo "Generating API documentation..."
	doxygen Doxyfile
	@echo "Done! API docs generated in website/api/"

serve-website: website
	@echo "Starting local server at http://localhost:8000"
	@cd website && python3 -m http.server 8000

# Help
help:
	@echo "ClankerOS Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build kernel (default)"
	@echo "  kernel       - Build kernel only"
	@echo "  programs     - Build userspace programs"
	@echo "  libraries    - Build system libraries"
	@echo "  clean        - Clean all build artifacts"
	@echo "  run          - Run kernel in QEMU"
	@echo "  run-serial   - Run kernel in QEMU with serial output"
	@echo "  debug        - Run kernel in QEMU with GDB server"
	@echo "  toolchain    - Build cross-compiler toolchain"
	@echo "  website      - Generate documentation website"
	@echo "  docs         - Generate Doxygen API documentation"
	@echo "  serve-website - Start local web server for website"
	@echo "  help         - Show this help message"
