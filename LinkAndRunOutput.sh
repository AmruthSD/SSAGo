#!/bin/bash
set -e  # stop on error

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"
INPUT_FILE="$ROOT_DIR/input.txt"
IR_FILE="$BUILD_DIR/output.ll"
RUNTIME_LIB="$BUILD_DIR/libruntime.a"
PROGRAM="$BUILD_DIR/program"

cmake --build "$BUILD_DIR"

"$BUILD_DIR/compiler" "$INPUT_FILE" "$IR_FILE"

# clang++ "$IR_FILE" "$RUNTIME_LIB" -o "$PROGRAM" -lboost_context

# "$PROGRAM"