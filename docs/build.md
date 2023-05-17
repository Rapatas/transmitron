# Build

The recommended build method is to use the provided docker images.

## Ubuntu 20.04 (and later)

```bash
git clone https://github.com/Rapatas/transmitron.git
cd transmitron
./docker/ubuntu-2004/build-image.sh
docker run --rm -it -v $PWD:/workspace rapatas_transmitron_linux_compiler bash
```

Then, in the container:

```bash
macro-make
cd build_rapatas_transmitron_ubuntu-2004_compiler
cpack
```

## Ubuntu 18.04

```bash
git clone https://github.com/Rapatas/transmitron.git
cd transmitron
./docker/ubuntu-1804/build-image.sh
docker run --rm -it -v $PWD:/workspace rapatas_transmitron_linux_compiler bash
```

Then, in the container:

```bash
macro-make
cd build_rapatas_transmitron_ubuntu-1804_compiler
cpack
```

## Windows

**Transmitron is cross-compiled from Linux to windows. You will need a Linux
host to run this image.**

```bash
git clone https://github.com/Rapatas/transmitron.git
cd transmitron
./docker/windows/build-image.sh
docker run --rm -it -v $PWD:/workspace rapatas_transmitron_windows_compiler bash
```

Then, in the container:

```bash
macro-make
cd build_rapatas_transmitron_windows_compiler
cpack
```
