# Build

The recommended build method is to use the provided docker images for [linux](../docker/linux-x86-64) and [windows](../docker/windows-x86-64).
The debug and release variants will bake the dependencies in the docker image.

## Linux

```bash
git clone https://github.com/Rapatas/transmitron.git
cd transmitron
./docker/linux-x86-64/compiler/build-image.sh
./docker/linux-x86-64/release/build-image.sh
docker run --rm -it -v $PWD:/workspace rapatas-transmitron-linux-x86-64-release bash
```

Then, in the container:

```bash
macro-make
cd build-rapatas-transmitron-linux-x86-64-release
cpack
```

## Windows

**Transmitron is cross-compiled from Linux to windows. You will need a Linux
host to run this image.**

```bash
git clone https://github.com/Rapatas/transmitron.git
cd transmitron
./docker/windows-x86-64/compiler/build-image.sh
./docker/windows-x86-64/release/build-image.sh
docker run --rm -it -v $PWD:/workspace rapatas-transmitron-windows-x86-64-release bash
```

Then, in the container:

```bash
macro-make
cd build-rapatas-transmitron-windows-x86-64-release
cpack
```
