# Transmitron

![Preview](https://i.imgur.com/1whe3Pf.png "2 connections, 4 subscriptions, homepage")

## Build

### Requirements

- [conan](https://conan.io/) package manager
- C++ 17
- libgtk2.0-dev (on Ubuntu 18.04)

### Ubuntu 18.04

**C++ 17**

```bash
sudo apt-get install -y gcc-8
```

**conan**

```bash
pip3 install conan --upgrade
conan config set general.revisions_enabled=1
conan remote add bincrafters https://bincrafters.jfrog.io/artifactory/api/conan/public-conan
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
conan install ../conan/ --build=missing
cmake -DCMAKE_MODULE_PATH=$PWD -DCMAKE_BUILD_TYPE=Release ..
make -j $(nproc)
```

## Acknowledgements

- Icons made by [DinosoftLabs](https://www.flaticon.com/authors/DinosoftLabs), [Google](https://www.flaticon.com/authors/google), [Freepik](https://www.freepik.com), [Kiranshastry](https://www.flaticon.com/authors/kiranshastry) from [Flaticon](https://www.flaticon.com/).
