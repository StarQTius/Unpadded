#!/bin/sh -l

cd /home/ubuntu

export CLICOLOR_FORCE=1

cmake --preset gcc -S "$GITHUB_WORKSPACE" -B build \
&& cmake --build build --target check_full --parallel $(nproc) \
&& cmake --preset clang -S "$GITHUB_WORKSPACE" -B build_clang \
&& cmake --build build_clang --target check_full --parallel $(nproc) \
&& cmake --build build_clang --target clang_format --parallel $(nproc) \
&& cmake --build build_clang --target iwyu --parallel $(nproc) \
&& cmake --build build_clang --target clang_tidy --parallel $(nproc)

