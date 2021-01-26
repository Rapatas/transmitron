# Transmitron

## Build

Building requires the [conan](https://conan.io/) package manager.

```bash
mkdir build && cd build
conan install ../conan/
cmake -DCMAKE_TOOLCHAIN_FILE=conan_paths.cmake -DCMAKE_BUILD_TYPE=Release ..
make -j $(nproc)
```
