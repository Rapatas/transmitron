#!/bin/sh

set -eu

dir=$(dirname "$0")
docker build \
  -t rapatas_transmitron_windows_compiler \
  "$dir"
