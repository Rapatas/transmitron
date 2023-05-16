FROM ubuntu:22.04

USER root
WORKDIR /workspace
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update

COPY ./cmake-installer.sh /workspace
RUN /workspace/cmake-installer.sh

RUN true \
  && apt-get update \
  && apt-get install -y \
    mingw-w64 \
    git \
    python3 \
    python3-pip \
    nsis

RUN true \
  && pip3 install conan==1.60.0 \
  && conan remote add bincrafters https://bincrafters.jfrog.io/artifactory/api/conan/public-conan \
  && conan config set general.revisions_enabled=1 \
  && conan config set tools.system.package_manager:mode=install

COPY ./conan.profile /workspace
COPY ./conanfile.py  /workspace
RUN true \
  && mkdir -p /root/.conan/profiles \
  && cp /workspace/conan.profile /root/.conan/profiles/windows \
  && conan profile new --force default --detect \
  && conan install /workspace --build=missing --profile:build=default --profile:host=windows

RUN echo "rapatas_transmitron_windows_compiler" > /etc/compilername
COPY ./toolchain.cmake /opt/cmake-toolchains/linux-x86_64-to-windows-x86_64.cmake
COPY ./macro-make  /usr/local/bin
