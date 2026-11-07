#!/usr/bin/env bash
set -euo pipefail
cmake_command=(cmake)
build_dir="build/reference-math-contracts-${TARGET:?}-${ARCHITECTURE:?}-${CONFIGURATION:?}"
args=(-G Ninja -S reference-math -B "$build_dir"
  "-DCMAKE_BUILD_TYPE=${CONFIGURATION:?}" -DEVE_REFERENCE_MATH_BUILD_TESTS=ON)
case "${TARGET:?}" in
  win32)
    if [ "${ARCHITECTURE:?}" = arm64 ]; then
      cmake_command=(cmake/with-msvc.cmd --arm64 cmake)
    else
      cmake_command=(cmake/with-msvc.cmd cmake)
    fi ;;
  linux) args+=(-DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++) ;;
  macosx) args+=("-DCMAKE_OSX_ARCHITECTURES=${ARCHITECTURE:?}" -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0) ;;
  *) echo "Native numerical contracts cannot run on target: $TARGET" >&2; exit 1 ;;
esac
"${cmake_command[@]}" "${args[@]}"
"${cmake_command[@]}" --build "$build_dir" --parallel 4
ctest --test-dir "$build_dir" --output-on-failure
