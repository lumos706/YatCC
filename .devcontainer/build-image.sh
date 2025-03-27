#!/bin/bash
set -e
cd $(dirname "$0")

../antlr/setup.sh
../llvm/setup.sh

if [ -z "$YatCC_ANTLR_DIR" ]; then
  YatCC_ANTLR_DIR=$(realpath ../antlr)
fi
if [ -z "$YatCC_LLVM_DIR" ]; then
  YatCC_LLVM_DIR=$(realpath ../llvm)
fi
rm -rf antlr llvm

mkdir antlr
cp -rlP $YatCC_ANTLR_DIR/antlr.jar antlr/antlr.jar
cp -rlP $YatCC_ANTLR_DIR/source antlr/source
cp -rlP $YatCC_ANTLR_DIR/install antlr/install

_llvm_install=$YatCC_LLVM_DIR/install
mkdir llvm llvm/install llvm/install/bin llvm/install/lib
cp -rlP $YatCC_LLVM_DIR/llvm llvm/llvm
cp -rlP $YatCC_LLVM_DIR/clang llvm/clang
cp -rlP $_llvm_install/include llvm/install/include
cp -rlP -t llvm/install/bin \
  $_llvm_install/bin/clang-18 \
  $_llvm_install/bin/clang \
  $_llvm_install/bin/clang++ \
  $_llvm_install/bin/llc \
  $_llvm_install/bin/llvm-as \
  $_llvm_install/bin/llvm-dis \
  $_llvm_install/bin/lli \
  $_llvm_install/bin/opt \
  $_llvm_install/bin/llvm-config
cp -rlP -t llvm/install/lib \
  $_llvm_install/lib/libclang-cpp*.so* \
  $_llvm_install/lib/libLLVM*.so* \
  $_llvm_install/lib/clang
cp -rlP $_llvm_install/libexec llvm/install/libexec
cp -rlP $_llvm_install/share llvm/install/share

_version=$(date -u +%Y%m%dT%H%M%SZ)
buildah build --layers -f yatcc.Containerfile --target base -t yatcc:base .
buildah tag yatcc:base yatcc:base.$_version
buildah build --layers -f yatcc.Containerfile --target full -t yatcc:full .
buildah tag yatcc:full yatcc:full.$_version
buildah tag yatcc:full yatcc:dc.$_version
buildah tag yatcc:full yatcc:dc.latest
buildah tag yatcc:full yatcc:latest
