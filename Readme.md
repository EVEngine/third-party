# EVEngine third-party independent releases

Aggregate-owned dependency fixes are committed directly to their source files,
including SimpleSquirrel direct bindings (C++20). VKBuilder fixes are pinned in its
source submodule. Only external submodule fixes remain in `cmake/patches/`, applied
idempotently by `cmake/apply_patches.cmake`. EVEngine does not inject dependency
patches or generate replacement dependency implementations. Source and release
builds consume these same canonical headers and ABI defaults.

Native binding regression: `cmake -S tests/simplesquirrel -B build/bindings`,
then `cmake --build build/bindings` and `ctest --test-dir build/bindings --output-on-failure`.

## Version policy

Versions describe the whole dependency bundle independently of EVEngine.
Start at v0.1.0. Patch releases fix compatible source/packaging issues; during 0.x,
minor releases may upgrade dependencies or change ABI/toolchain/platform baselines.
Use 1.0.0 once the package format and compatibility policy are stable. From 1.x,
incompatible ABI or baseline changes require a major version. Never move a released
tag or overwrite assets; update the version for changed binaries.

## Publish a version

After reviewing and validating a dependency commit, create a new immutable tag:

```sh
git tag v0.1.0 <reviewed-dependency-commit>
git push origin v0.1.0
```

`.github/workflows/release.yml` builds Windows x64/ARM64 Release/Debug, Linux x86_64/ARM64,
macOS arm64/x86_64, Android arm64-v8a and iOS arm64 device installs in parallel.
It creates the matching GitHub Release and uploads ten `eve-third-party-*.zip`
files and `SHA256SUMS` only after all builds and installed-package tests pass.
Pull requests and `codex/release-*` candidate branches run the same build/package
checks without publishing. Manual dispatch
accepts an existing tag and checks that checkout HEAD equals that tag's commit.
Assets are never overwritten: changes require a new tag/version. A publishing retry
with existing assets fails explicitly instead of replacing another binary version.

EVEngine pins this tag AND its exact commit in `cmake/third_party_pin.cmake`.
Advance those pins only after all assets exist; no dependency build is run during
ordinary EVEngine native builds or SDK releases. First publish the initial version
before enabling its pins downstream. To roll back, restore both pins together.

Each zip is an install prefix with include/, lib/, bin/ (when produced), license
notices, `third-party-version.txt` and `eve-third-party-package.json`. Manifest
schema `evengine.third-party-package` version 1 records the tag, aggregate commit,
platform, architecture and configuration. Unknown v1 fields are ignored; unknown
schema versions are rejected. This initial schema has no legacy migrations.
Consumers verify SHA256, metadata and install completeness before atomically
publishing a directory; an incompatible existing prefix is never overwritten.

## Binary compatibility

Windows uses MSVC on windows-2022 (x64) or windows-11-arm (ARM64), /MD or /MDd.
Linux uses Ubuntu 24.04/GCC on native x86_64 or ARM64 runners.
macOS uses macos-15 (arm64) and macos-15-intel (x86_64), deployment target 13.0.
Android uses NDK 27.3.13750724, API 24, arm64-v8a, c++_shared. iOS uses arm64 device
SDK, deployment target 13.0. Native packages include static and shared SDL/Box2D/
Box3D so the engine's OBJECT and SHARED routes can reuse the same install.
Jolt is static, double precision, no RTTI, and installed as `Jolt::Jolt`.
Unsupported targets (Emscripten, iOS simulator, other ABIs, custom instrumentation)
continue to use source builds; there is no binary-download failure fallback.

## Local build

```sh
git submodule update --init --recursive
TARGET=linux ARCHITECTURE=x86_64 CONFIGURATION=Release bash scripts/build-release.sh
TARGET=linux CONFIGURATION=Release bash scripts/test-installed.sh
python3 scripts/package.py linux v0.1.0 --architecture x86_64
```

On Windows, run the script in Git Bash; `cmake/with-msvc.cmd` initializes MSVC.
Install Ninja and yasm (`choco install ninja yasm`); mpg123's MSVC assembly
sources require yasm, which is not preinstalled on the hosted Windows runner.
ARM64 Windows builds and installed-package tests select `vcvarsarm64.bat` through
`cmake/with-msvc.cmd --arm64`; set `ARCHITECTURE=arm64` for both scripts.
Android additionally needs ANDROID_HOME and ANDROID_NDK_VERSION. Native builds
require their OS development packages and Vulkan headers/SDK.

## Jolt Physics

`jolt/` is the official Jolt Physics submodule, pinned to **v5.5.0**
(`23dadd0e603f1b321142d4c74df07fce85064989`, MIT; installed license included).
Initialize it with `git submodule update --init --recursive`.

The aggregate builds and installs Jolt by default. Set `EVENGINE_BUILD_JOLT=OFF`
to omit it. `cmake/Jolt.cmake` scopes upstream options to this dependency:
static library, dynamic MSVC CRT, double-precision positions, no IPO, no AVX/BMI
requirement, no profiler/debug renderer or floating-point traps. This adds the
library package; it does not enable a Jolt world in EVEngine yet.

Consumers must use `find_package(Jolt CONFIG REQUIRED)` and link `Jolt::Jolt`
from the install prefix. The imported target carries ABI defines (notably
`JPH_DOUBLE_PRECISION`), include paths, configuration-specific archive names and
thread/compiler requirements. Do not link a bare archive without these usage
requirements. Keep runtime initialization and static Jolt symbols in one engine
link group; public engine interfaces must not expose Jolt types.

Focused package validation (GCC/Clang, or MSVC through a Developer shell):

```sh
cmake -S tests/jolt -B build/jolt-check -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$PWD/build/jolt-install"
cmake --build build/jolt-check --parallel 4
ctest --test-dir build/jolt-check --output-on-failure
cmake --install build/jolt-check
cmake -S tests/jolt -B build/jolt-consumer -DEVENGINE_JOLT_USE_INSTALLED=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$PWD/build/jolt-install"
cmake --build build/jolt-consumer --parallel 4
ctest --test-dir build/jolt-consumer --output-on-failure
cmake -S tests/jolt -B build/jolt-absent -DEVENGINE_BUILD_JOLT=OFF
```

The smoke test runs upstream's falling-body example both in-tree and through the
installed package; configure also checks parent-option isolation and ABI exports.

## ARM64 原生构建 / Native ARM64 builds

先运行 `git submodule update --init --recursive`。需要 CMake >= 3.21、Ninja
及原生 ARM64 C/C++ 工具链。Windows 安装 Visual Studio ARM64 C++ 工具与
Windows SDK；Linux 安装 aarch64 GCC/G++ 和依赖库所需的系统开发包。

Windows ARM64（PowerShell / cmd）：

```powershell
cmake\with-msvc.cmd --arm64 cmake --preset windows-arm64-release
cmake\with-msvc.cmd --arm64 cmake --build --preset windows-arm64-release
```

Linux ARM64：

```sh
cmake --preset linux-arm64-release
cmake --build --preset linux-arm64-release
```

Debug 将命令中的 `release` 换为 `debug`。四种配置分别使用
`build/<preset>` 构建目录，安装到 `build/install/<preset>`，互不混用，
也不会覆盖原有 x64 库。构建 preset 默认执行 `install`，并发为 4；
可用 `cmake --build --preset <preset> -j 8` 覆盖。

这些配置检查宿主、目标和编译器的 ARM64 架构，不用于交叉编译。
直接配置也可传 `-DEVENGINE_NATIVE_ARM64=ON`，并指定独立的 `-B` 与
`-DCMAKE_INSTALL_PREFIX`。未开启此选项时保留原有构建行为。

The four presets build and install native Windows/Linux ARM64 dependencies.
Use the ARM64 MSVC wrapper for both Windows configure and build commands;
Linux uses the native GCC/G++ environment. Replace `release` with `debug` for
Debug libraries. Each preset has separate build and install directories under
`build/`. The native architecture check rejects x64 compilers and cross builds.
EVEngine's Makefile manages its own architecture-specific dependency directories;
these presets are the standalone entry points for this repository.
