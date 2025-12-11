#!/bin/bash
# ClankerOS Toolchain Build Script
# Builds i386-elf cross-compiler (GCC + binutils)

set -e

# Configuration
TARGET=i386-elf
TOOLCHAIN_DIR="$(dirname "$0")/../toolchain"
PREFIX="$(cd "$TOOLCHAIN_DIR" && pwd)"
BINUTILS_VERSION=2.41
GCC_VERSION=13.2.0

# Parallel build jobs
JOBS=$(nproc)

echo "Building ClankerOS i386 cross-compiler toolchain..."
echo "Target: $TARGET"
echo "Install prefix: $PREFIX"
echo ""

# Create build directories
mkdir -p /tmp/clankeros-toolchain-build/{binutils,gcc}
cd /tmp/clankeros-toolchain-build

# Download sources if needed
if [ ! -f "binutils-${BINUTILS_VERSION}.tar.xz" ]; then
    echo "Downloading binutils ${BINUTILS_VERSION}..."
    wget "https://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.xz"
fi

if [ ! -f "gcc-${GCC_VERSION}.tar.xz" ]; then
    echo "Downloading GCC ${GCC_VERSION}..."
    wget "https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.xz"
fi

# Extract sources
if [ ! -d "binutils-${BINUTILS_VERSION}" ]; then
    echo "Extracting binutils..."
    tar -xf "binutils-${BINUTILS_VERSION}.tar.xz"
fi

if [ ! -d "gcc-${GCC_VERSION}" ]; then
    echo "Extracting GCC..."
    tar -xf "gcc-${GCC_VERSION}.tar.xz"

    # Download GCC prerequisites
    cd "gcc-${GCC_VERSION}"
    ./contrib/download_prerequisites
    cd ..
fi

# Build binutils
echo ""
echo "Building binutils..."
cd binutils
../binutils-${BINUTILS_VERSION}/configure \
    --target=$TARGET \
    --prefix="$PREFIX" \
    --with-sysroot \
    --disable-nls \
    --disable-werror

make -j"$JOBS"
make install
cd ..

# Add to PATH for GCC build
export PATH="$PREFIX/bin:$PATH"

# Build GCC
echo ""
echo "Building GCC..."
cd gcc
../gcc-${GCC_VERSION}/configure \
    --target=$TARGET \
    --prefix="$PREFIX" \
    --disable-nls \
    --enable-languages=c,c++ \
    --without-headers

make -j"$JOBS" all-gcc
make -j"$JOBS" all-target-libgcc
make install-gcc
make install-target-libgcc
cd ..

echo ""
echo "Toolchain build complete!"
echo "Toolchain installed to: $PREFIX"
echo "Add the following to your PATH:"
echo "export PATH=\"$PREFIX/bin:\$PATH\""
echo ""
echo "Verify installation:"
echo "$PREFIX/bin/$TARGET-gcc --version"
