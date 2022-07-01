#!/bin/sh

set -eu

dir=$(dirname "$0")
docker build \
  -t rapatas_transmitron_linux_compiler \
  "$dir"
