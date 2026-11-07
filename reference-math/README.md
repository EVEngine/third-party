# EVEngine procedural reference math

This package compiles unchanged, pinned V8 numerical source and CORE-MATH pow.
UPSTREAM.json is the provenance ledger with upstream paths, revisions and SHA256s.
No JavaScript runtime is embedded. The small C ABI exports twelve value-only binary64
functions, hiding vendor headers from EVEngine, SDK clients and module boundaries.

These are inexpensive, stateless scalar calls: no ownership, allocation, callbacks,
locks, named RNG, clock, persistent format or mutable registry. Calls are thread safe
under the caller's normal FE_TONEAREST environment; ordinary floating exception flags
may change. pow includes ECMAScript NaN/exponent special cases. Canonical CORE-MATH
rounding is not universally bitwise identical to the selected Node system pow; retain
explicit per-effect upstream comparisons rather than inferring image parity from scalar
accuracy or changing expected hashes.

Standalone source build: cmake -S reference-math -B build/reference-math followed by
cmake --build and cmake --install. Enable EVE_REFERENCE_MATH_BUILD_TESTS for CTest
contracts. Windows configure/build commands must run in the MSVC Developer environment
(the aggregate provides cmake/with-msvc.cmd), and source builds additionally require clang-cl on PATH for the original
CORE-MATH C implementation; V8 and the public adapter keep the aggregate's MSVC C++
compiler and DLL CRT. Prebuilt/SDK consumers require no Clang compiler or vendor headers.
No source patching, header replacement, std symbol interception or numerical fallback
is used. All targets use strict binary64 operations and PIC archives. Release uses optimized
math; MSVC Debug preserves its default runtime-check-compatible C++ optimization profile.

Consumers use find_package(EveReferenceMath CONFIG REQUIRED), link
EveReferenceMath::eve_reference_math and include <eve/reference_math.h>. The same
archive, public header, CMake export and license/provenance files are installed by the
aggregate and standalone build. The aggregate decides platform/debug library postfixes.
The package currently has native Windows-x64/MSVC-consumer and Linux/GCC experimental
proof. Other named targets need actual build/runtime evidence before support is claimed.

The provenance ledger has schema evengine.reference-math.vendor-provenance v1.
Unknown schema/version/provider names reject; additional descriptive metadata is ignored.
Configuration verifies every recorded vendor source hash. Vendor bytes are imported
directly from the pinned Git blobs and are marked -text to prevent checkout conversion.

Adapter/build code follows the existing EVEngine dual license; see LICENSE and its
two referenced texts. Vendored V8 and CORE-MATH files keep their original licenses,
including all source notices; the full preserved snapshot is installed under vendor/.

Connection choice: one compile-known provider, per-pixel call frequency, no observers,
and atomic scalar results use direct typed calls. No capability lookup or string dispatch
occurs in the numerical hot path. There is no ECS, handle lifetime, time/RNG owner,
callback registration, persisted gameplay state or optional runtime fallback in this package.

Native dependency CI runs scripts/test-reference-math.sh for source contracts and
scripts/test-installed.sh for the actual aggregate install. The installed consumer
uses find_package(EveReferenceMath CONFIG REQUIRED), repeats the numerical contracts
through installed headers/archives only, and compiles a separate C translation unit.
Its C-compatible header does not expose vendor headers; the archive retains its C++
link requirement. These jobs cover the existing native matrix and do not by themselves
prove Android/iOS runtime behavior or authorize a dependency release.

Windows CORE-MATH object architecture is selected by compiling target-macro probes
with the configured C++ compiler, never from host processor metadata alone. Only
x64 and ARM64 compiler targets are accepted; ARM64EC and 32-bit targets reject.
Source/installed native matrix evidence is still required for each named platform.
