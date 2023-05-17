#!/bin/sh

set -e

[ $(id -u) -ne 0 ] && SUDO=sudo

version=3.26.3

$SUDO apt-get install -y openssl libssl-dev build-essential wget

cd /tmp/

wget https://github.com/Kitware/CMake/releases/download/v$version/cmake-$version.tar.gz
tar -xzf cmake-$version.tar.gz

cd cmake-$version
# Enable openssl
sed -i 's/cmake_options="-DCMAKE_BOOTSTRAP=1"/cmake_options="-DCMAKE_BOOTSTRAP=1 -DCMAKE_USE_OPENSSL=ON"/' bootstrap

mkdir build && cd build
../bootstrap --parallel=$(nproc)
make -j $(nproc)
$SUDO make install
