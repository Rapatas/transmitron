FROM ubuntu:18.04

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
  && ./llvm.sh 17 \
  && apt-get install -y \
    clang-tidy-17 \
    clang-format-17 \
  && update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang-17 50 --force \
  && update-alternatives --install /usr/bin/cc  cc  /usr/bin/clang-17 50 --force

COPY ./update-alternatives-clang.sh /workspace/update-alternatives-clang.sh
RUN /workspace/update-alternatives-clang.sh 17 17

RUN true \
  && apt-get install -y python3 python3-pip \
  && pip3 install conan==1.62.0

RUN true \
  && apt-get update \
  && apt-get install -y \
    curl \
    wget \
    vim \
    tree \
    unzip \
    libgtk-3-dev \
    libgl1-mesa-dev \
    libegl1-mesa-dev \
    libpng-dev \
    libjpeg-dev

RUN mkdir -p /root/.conan/profiles
COPY ./conan.profile /root/.conan/profiles/default

RUN echo "rapatas-transmitron-linux-x86-64" > /etc/compilername
