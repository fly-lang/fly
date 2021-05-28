#!/bin/sh

CMAKE_COMMAND="cmake -G Ninja -DCMAKE_BUILD_TYPE=Release " \
    "-DLLVM_TARGETS_TO_BUILD=X86"