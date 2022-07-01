# Build

The recommended build method is to use the provided docker images:

## Linux (Debian)

```bash
git clone https://github.com/Rapatas/transmitron.git
cd transmitron
./docker/linux/build-image.sh
docker run --rm -it -v $PWD:/workspace rapatas_transmitron_linux_compiler bash
```

Then, in the container:

```bash
macro-make
cd build_rapatas_transmitron_linux_compiler
make install DESTDIR=./installed
```

Now you can launch it on the host with:

```bash
./build_rapatas_transmitron_linux_compiler/installed/usr/local/bin/transmitron
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
make package
```
