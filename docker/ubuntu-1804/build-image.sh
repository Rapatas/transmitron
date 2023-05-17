#!/bin/sh

set -eu

script_dir=$(dirname "$0")
temp_dir=$(mktemp -d)

echo "Using temp dir $temp_dir"

cp -r "$script_dir"/* "$temp_dir"
cp "$script_dir/../../conan/conanfile.py" "$temp_dir/"
cp "$script_dir/../../resources/cmake-installer.sh" "$temp_dir/"
cp "$script_dir/../../resources/update-alternatives-clang.sh" "$temp_dir/"

docker build \
  -t rapatas_transmitron_ubuntu-1804_compiler \
  "$temp_dir"
