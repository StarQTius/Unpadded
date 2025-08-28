#!/bin/sh -l

cd /home/ubuntu

export CLICOLOR_FORCE=1

BUILD_DIR="$GITHUB_WORKSPACE/build"
BUILD_CLANG_DIR="$GITHUB_WORKSPACE/build_clang"

sudo mkdir "$BUILD_DIR" "$BUILD_CLANG_DIR"
sudo chown ubuntu:ubuntu "$BUILD_DIR" "$BUILD_CLANG_DIR"

cmake --preset gcc -S "$GITHUB_WORKSPACE" -B "$BUILD_DIR"  \
&& cmake --build "$BUILD_DIR" --target check_full --parallel $(nproc) \
&& cmake --preset clang -S "$GITHUB_WORKSPACE" -B "$BUILD_CLANG_DIR" \
&& cmake --build "$BUILD_CLANG_DIR" --target check_full --parallel $(nproc) \
&& cmake --build "$BUILD_CLANG_DIR" --target clang_format --parallel $(nproc) \
&& cmake --build "$BUILD_CLANG_DIR" --target iwyu --parallel $(nproc) \
&& cmake --build "$BUILD_CLANG_DIR" --target clang_tidy --parallel $(nproc)

