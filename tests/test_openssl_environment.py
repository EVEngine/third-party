"""Exercise canonical OpenSSL commands with Windows and POSIX PATH values."""

import os
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


class OpenSSLEnvironmentTests(unittest.TestCase):
    def run_case(self, path):
        windows = os.name == "nt"
        cmake = Path(shutil.which("cmake")).resolve().as_posix()
        # Use native Perl on Windows: MSYS may translate environment values
        # before the fixture can inspect what cmake -E env actually supplied.
        native_perl = Path("C:/Strawberry/perl/bin/perl.exe")
        perl = (
            (
                native_perl
                if os.name == "nt" and native_perl.is_file()
                else Path(shutil.which("perl"))
            )
            .resolve()
            .as_posix()
        )
        make = Path(shutil.which("make") or shutil.which("gmake")).resolve().as_posix()
        with tempfile.TemporaryDirectory(prefix="openssl env ") as temporary:
            # Windows TEMP may contain RUNNER~1 while CMake expands that
            # short name. Compare canonical paths, preserving all PATH bytes.
            root = Path(temporary).resolve()
            (root / "cmake").mkdir()
            source = root / "openssl"
            source.mkdir()
            # Unmodified production module; only the external build tool is a
            # fixture. No OpenSSL download, compilation or repository mutation.
            shutil.copyfile(ROOT / "cmake/OpenSSL.cmake", root / "cmake/OpenSSL.cmake")
            (source / "VERSION.dat").write_text(
                "MAJOR=3\nMINOR=0\nPATCH=0\n", encoding="utf-8"
            )
            tools = Path(cmake).parent.as_posix()
            expected = tools + (";" if windows else ":") + path
            (root / "expected-path").write_text(expected, encoding="utf-8")
            (root / "expected-ndk").write_text(
                (root / "NDK with spaces").as_posix(), encoding="utf-8"
            )
            (source / "Configure").write_text(
                r"""use strict;
use warnings;
use FindBin;
for my $pair (["PATH", "expected-path"], ["ANDROID_NDK_ROOT", "expected-ndk"]) {
    open my $file, '<', "$FindBin::Bin/../$pair->[1]" or die $!;
    local $/;
    my $expected = <$file>;
    die "incorrect $pair->[0]: actual=[$ENV{$pair->[0]}] expected=[$expected]"
        unless $ENV{$pair->[0]} eq $expected;
}
die "missing Android target" unless $ARGV[0] eq "android-arm64";
open my $marker, '>', "$FindBin::Bin/../configure.ok" or die $!;
print $marker "ok";
""",
                encoding="utf-8",
            )
            for stage in ("build", "install"):
                (root / f"{stage}.cmake").write_text(
                    """file(READ "${CMAKE_CURRENT_LIST_DIR}/expected-path" expected)
if(NOT "$ENV{PATH}" STREQUAL "${expected}")
    message(FATAL_ERROR "PATH was split or changed")
endif()
file(READ "${CMAKE_CURRENT_LIST_DIR}/expected-ndk" expected_ndk)
if(NOT "$ENV{ANDROID_NDK_ROOT}" STREQUAL "${expected_ndk}")
    message(FATAL_ERROR "NDK root was split or changed")
endif()
file(WRITE "${CMAKE_CURRENT_LIST_DIR}/"""
                    + stage
                    + """.ok" "ok")
""",
                    encoding="utf-8",
                )
            build = root / "build"
            working = build / "eve_openssl-prefix/src/eve_openssl-build"
            working.mkdir(parents=True)
            (working / "Makefile").write_text(
                f'build_libs:\n\t"{cmake}" -P "{root.as_posix()}/build.cmake"\n'
                f'install_dev:\n\t"{cmake}" -P "{root.as_posix()}/install.cmake"\n',
                encoding="utf-8",
            )
            (root / "CMakeLists.txt").write_text(
                f'''cmake_minimum_required(VERSION 3.21)
project(OpenSSLEnvironment NONE)
set(CMAKE_BUILD_TYPE Release)
set(ANDROID TRUE)
set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
set(ANDROID_PLATFORM_LEVEL 24)
set(CMAKE_ANDROID_NDK "${{CMAKE_CURRENT_SOURCE_DIR}}/NDK with spaces")
set(CMAKE_C_COMPILER "{cmake}")
# The fixture exercises process environments, with no target compilation.
set(CMAKE_C_COMPILER_LOADED TRUE)
set(CMAKE_HAVE_LIBC_PTHREAD TRUE)
set(PERL_EXECUTABLE "{perl}")
set(_make "{make}")
set(ENV{{PATH}} "{path}")
include(cmake/OpenSSL.cmake)
eve_add_openssl()
''',
                encoding="utf-8",
            )
            for command in (
                [cmake, "-S", str(root), "-B", str(build), "-G", "Ninja"],
                [cmake, "--build", str(build), "--target", "eve_openssl"],
            ):
                result = subprocess.run(
                    command,
                    capture_output=True,
                    text=True,
                    encoding="utf-8",
                    env=os.environ.copy(),
                    check=False,
                )
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            for stage in ("configure", "build", "install"):
                self.assertEqual(
                    (root / f"{stage}.ok").read_text(encoding="utf-8"), "ok"
                )

    def test_native_path_survives_all_external_project_stages(self):
        self.run_case(
            "C:/Git tools/bin;C:/Git/usr/bin"
            if os.name == "nt"
            else "/tools with spaces/bin:/other/bin"
        )

    def test_semicolons_and_spaces_survive_all_external_project_stages(self):
        # Exercise the actual host process ABI. GNU Make on Windows rewrites
        # POSIX PATH strings; pretending that host is POSIX tests Make rather
        # than the environment supplied by our ExternalProject commands.
        self.run_case(
            "C:/Git tools/bin;D:/other tools/bin;C:/third/bin"
            if os.name == "nt"
            else "/tools with spaces/bin:/other;literal/bin"
        )


if __name__ == "__main__":
    unittest.main(verbosity=2)
