#!/bin/bash
set -e # Exit on error
if [ -z "$YatCC_ANTLR_DIR" ]; then
  cd $(dirname "$0")
else
  mkdir -p "$YatCC_ANTLR_DIR"
  cd "$YatCC_ANTLR_DIR"
fi

if [ -d install ]; then
  echo "ALREADY SETUP! - please remove the install directory to reinstall:"
  echo "  rm -rf $(realpath install)"
  exit 0
fi
rm -rf source build install

if [ ! -f antlr.jar ]; then
  wget -O antlr.jar https://www.antlr.org/download/antlr-4.13.2-complete.jar
fi
if [ ! -f cpp-runtime.zip ]; then
  wget -O cpp-runtime.zip https://www.antlr.org/download/antlr4-cpp-runtime-4.13.2-source.zip
fi

unzip cpp-runtime.zip -d source
mkdir build install
cmake source -B build -G Ninja \
  -DCMAKE_INSTALL_PREFIX=$(realpath install) \
  -DANTLR4_INSTALL=ON
cmake --build build --target install
