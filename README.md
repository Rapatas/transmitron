# Transmitron

![Preview](https://i.imgur.com/1gXWY8T.png "2 connections, 3 subscriptions")

## Build

Building requires the [conan](https://conan.io/) package manager.

```bash
mkdir build && cd build
conan install ../conan/
cmake -DCMAKE_TOOLCHAIN_FILE=conan_paths.cmake -DCMAKE_BUILD_TYPE=Release ..
make -j $(nproc)
```
