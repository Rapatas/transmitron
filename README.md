# Transmitron

![Preview](https://i.imgur.com/eGsQBel.png "2 connections, 4 subscriptions, homepage")

## Build

### Requirements

- [conan](https://conan.io/) package manager
- C++ 17
- libgtk2.0-dev (on Ubuntu 18.04)

### Ubuntu 18.04

**C++ 17**

```bash
sudo apt-get install -y gcc-8

# Optionally if required:
sudo update-alternatives \
    --install /usr/bin/gcc gcc /usr/bin/gcc-7 70 \
    --slave /usr/bin/g++ g++ /usr/bin/g++-7
sudo update-alternatives \
    --install /usr/bin/gcc gcc /usr/bin/gcc-8 80 \
    --slave /usr/bin/g++ g++ /usr/bin/g++-8
sudo update-alternatives --config gcc
```

**conan**

```bash
pip3 install conan --upgrade
conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan
```

**libgtk2.0-dev**

```bash
sudo apt install -y libgtk2.0-dev
```

**transmitron**

```bash
git clone git@github.com:rapatas/transmitron.git
cd transmitron/

mkdir build && cd build
conan install ../conan/
cmake -DCMAKE_TOOLCHAIN_FILE=conan_paths.cmake -DCMAKE_BUILD_TYPE=Release ..
make -j $(nproc)
```

## Acknowledgements

- Icons made by [Google](https://www.flaticon.com/authors/google), [Freepik](https://www.freepik.com), [Kiranshastry](https://www.flaticon.com/authors/kiranshastry) from [Flaticon](https://www.flaticon.com/).