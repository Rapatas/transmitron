FROM ubuntu:20.04

USER root
WORKDIR /workspace
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update

COPY ./cmake-installer.sh /workspace
RUN /workspace/cmake-installer.sh

RUN true \
  && apt-get install -y lsb-release wget software-properties-common gnupg \
  && wget https://apt.llvm.org/llvm.sh \
  && chmod +x llvm.sh \
  && ./llvm.sh 16 \
  && apt-get install -y \
    clang-tidy-16 \
    clang-format-16 \
  && update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang-16 50 --force \
  && update-alternatives --install /usr/bin/cc  cc  /usr/bin/clang-16 50 --force

COPY ./update-alternatives-clang.sh /workspace/update-alternatives-clang.sh
RUN /workspace/update-alternatives-clang.sh 16 16

RUN true \
  && apt-get install -y python3 python3-pip \
  && pip3 install conan==1.60.0 \
  && conan remote add bincrafters https://bincrafters.jfrog.io/artifactory/api/conan/public-conan \
  && conan config set general.revisions_enabled=1 \
  && conan config set tools.system.package_manager:mode=install

RUN true \
  && apt-get update \
  && apt-get install -y \
    libgtk2.0-dev \
    libxcb-dri3-dev \
    libgl1-mesa-dev \
    xorg-dev \
    libx11-xcb-dev \
    libxcb-render0-dev \
    libxcb-render-util0-dev \
    libxcb-xkb-dev \
    libxcb-icccm4-dev \
    libxcb-image0-dev \
    libxcb-keysyms1-dev \
    libxcb-randr0-dev \
    libxcb-shape0-dev \
    libxcb-sync-dev \
    libxcb-xfixes0-dev \
    libxcb-xinerama0-dev \
    xkb-data \
    libxcb-util-dev \
    libsecret-1-dev

COPY ./conan.profile /workspace
COPY ./conanfile.py /workspace
RUN true \
  && mkdir -p /root/.conan/profiles \
  && cp /workspace/conan.profile /root/.conan/profiles/default \
  && conan install . --build=missing

RUN echo "rapatas_transmitron_ubuntu-2004_compiler" > /etc/compilername
COPY ./macro-make /usr/local/bin
