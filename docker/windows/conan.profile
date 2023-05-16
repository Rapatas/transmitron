toolchain=/usr/x86_64-w64-mingw32
target_host=x86_64-w64-mingw32

[env]
# CONAN_CMAKE_FIND_ROOT_PATH=$toolchain  # Optional, for CMake to find things in that folder
# CONAN_CMAKE_SYSROOT=$toolchain  # Optional, if we want to define sysroot

CHOST=$target_host
AR=$target_host-ar
AS=$target_host-as-posix
RANLIB=$target_host-ranlib
CC=$target_host-gcc-posix
CXX=$target_host-g++-posix
STRIP=$target_host-strip-posix
RC=$target_host-windres

[settings]
os=Windows
arch=x86_64
compiler=gcc
compiler.version=10
compiler.libcxx=libstdc++11
build_type=Release

[conf]
