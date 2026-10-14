#!/usr/bin/env bash
set -euo pipefail
cmake_command=(cmake)
if [ "${TARGET:?}" = win32 ]; then
  cmake_command=(cmake/with-msvc.cmd cmake)
fi
"${cmake_command[@]}" -G Ninja -S tests/jolt -B build/package-check \
  -DEVENGINE_JOLT_USE_INSTALLED=ON "-DCMAKE_PREFIX_PATH=$PWD/build/install" \
  "-DCMAKE_BUILD_TYPE=${CONFIGURATION:?}"
"${cmake_command[@]}" --build build/package-check --parallel 4
ctest --test-dir build/package-check --output-on-failure
