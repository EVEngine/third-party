这个是EVEngine的第三方库，可能有部分代码是被修改过的，在升级时要注意

master分支存放源代码
linux, darwin, win32, android, ios 等不同的二进制版本发布到github上供下载

在CMake中使用第三方下载安装的指令，将代码下载下来编译，并安装到third-party/binary下


推荐在不同平台下编译，然后将对应二进制库存放到github release附件中使用，在CMake中可选是使用预编译的版本还是下载源代码编译

每个third-party的release版，要打tag，方便主仓库用命令下载之前版本或者回滚测试。
不使用submodule因为这个第三方库列表可能会频繁修改，打补丁或升降级，普通的引擎开发者只需要关注核心代码，无需考虑修改第三方库

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
