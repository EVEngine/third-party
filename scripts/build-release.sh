#!/usr/bin/env bash
set -euo pipefail
target="${TARGET:?}"
configuration="${CONFIGURATION:?}"
architecture="${ARCHITECTURE:?}"
cmake_command=(cmake)
args=(-G Ninja -S . -B build/deps "-DCMAKE_BUILD_TYPE=$configuration"
  "-DCMAKE_INSTALL_PREFIX=$PWD/build/install" -DEVENGINE_DEPS_RELEASE=ON
  -DBUILD_TESTING=OFF -DEVENGINE_BUILD_JOLT=ON)
case "$target" in
  win32) cmake_command=(cmake/with-msvc.cmd cmake) ;;
  linux) args+=(-DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++) ;;
  macosx) args+=("-DCMAKE_OSX_ARCHITECTURES=$architecture" -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0) ;;
  android)
    ndk="${ANDROID_HOME:?}/ndk/${ANDROID_NDK_VERSION:?}"
    args+=("-DCMAKE_TOOLCHAIN_FILE=$ndk/build/cmake/android.toolchain.cmake"
      -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-24 -DANDROID_STL=c++_shared) ;;
  ios) args+=("-DCMAKE_TOOLCHAIN_FILE=$PWD/cmake/ios.toolchain.cmake" -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0) ;;
  *) echo "Unsupported target: $target" >&2; exit 1 ;;
esac
cmake -P cmake/apply_patches.cmake
cmake -P cmake/apply_patches.cmake # Reconfiguration must remain idempotent.
"${cmake_command[@]}" "${args[@]}"
"${cmake_command[@]}" --build build/deps --target install --parallel 4
cmake "-DPREFIX=$PWD/build/install" "-DTP_DIR=$PWD" "-DPLATFORM=$target" \
  "-DBUILD_TYPE=$configuration" -P cmake/write_version.cmake
