#!/bin/bash
set -e # Exit on error
if [ -z "$YatCC_LLVM_DIR" ]; then
  cd $(dirname "$0")
else
  mkdir -p "$YatCC_LLVM_DIR"
  cd "$YatCC_LLVM_DIR"
fi

if [ -d install ]; then
  echo "ALREADY SETUP! - please remove the install directory to reinstall:"
  echo "  rm -rf $(realpath install)"
  exit 0
fi
rm -rf llvm clang cmake build install

arch=$(uname -m)
if [ "$arch" = "x86_64" ]; then
  targets="X86"
elif [ "$arch" = "aarch64" ]; then
  targets="AArch64"
else
  echo "unsupported architecture: $arch"
  exit 1
fi

if [ ! -f llvm-18.src.tar.xz ]; then
  wget -O llvm-18.src.tar.xz https://github.com/llvm/llvm-project/releases/download/llvmorg-18.1.8/llvm-18.1.8.src.tar.xz
fi
if [ ! -f clang-18.src.tar.xz ]; then
  wget -O clang-18.src.tar.xz https://github.com/llvm/llvm-project/releases/download/llvmorg-18.1.8/clang-18.1.8.src.tar.xz
fi
if [ ! -f cmake-18.src.tar.xz ]; then
  wget -O cmake-18.src.tar.xz https://github.com/llvm/llvm-project/releases/download/llvmorg-18.1.8/cmake-18.1.8.src.tar.xz
fi

tar -xJvf llvm-18.src.tar.xz
mv llvm-18*.src llvm
tar -xJvf clang-18.src.tar.xz
mv clang-18*.src clang
tar -xJvf cmake-18.src.tar.xz
mv cmake-18*.src cmake

mkdir build install
cmake llvm -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_INSTALL_PREFIX=$(realpath install) \
  -DLLVM_ENABLE_PROJECTS="clang" \
  -DLLVM_TARGETS_TO_BUILD="$targets" \
  -DLLVM_BUILD_LLVM_DYLIB=ON \
  -DLLVM_LINK_LLVM_DYLIB=ON \
  -DLLVM_USE_LINKER=lld \
  -DLLVM_INCLUDE_BENCHMARKS=OFF \
  -DLLVM_INCLUDE_EXAMPLES=OFF \
  -DLLVM_INCLUDE_TESTS=OFF
cmake --build build --target install
