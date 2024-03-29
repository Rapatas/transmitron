[settings]
os=Windows
arch=x86_64
compiler=gcc
compiler.version=10
compiler.libcxx=libstdc++11
build_type=Debug

[options]
[build_requires]

[env]
LDFLAGS=-static -static-libstdc++

AR=x86_64-w64-mingw32-ar
AS=x86_64-w64-mingw32-as-posix
CC=x86_64-w64-mingw32-gcc-posix
CHOST=x86_64-w64-mingw32
CXX=x86_64-w64-mingw32-g++-posix
RANLIB=x86_64-w64-mingw32-ranlib
RC=x86_64-w64-mingw32-windres
STRIP=x86_64-w64-mingw32-strip-posix

[conf]
tools.build:compiler_executables = { "cpp": "x86_64-w64-mingw32-g++-posix", "c": "x86_64-w64-mingw32-gcc-posix" }

