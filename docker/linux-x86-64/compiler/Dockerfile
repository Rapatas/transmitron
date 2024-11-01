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
  && ./llvm.sh 18 \
  && apt-get install -y \
    clang-tidy-18 \
    clang-format-18 \
  && update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang-18 180 --force \
  && update-alternatives --install /usr/bin/cc  cc  /usr/bin/clang-18 180 --force

COPY ./update-alternatives-clang.sh /workspace/update-alternatives-clang.sh
RUN /workspace/update-alternatives-clang.sh 18 18

RUN true \
  && apt-get install -y python3.8 python3-pip \
  && update-alternatives --install /usr/bin/python3  python3 /usr/bin/python3.8 308 --force \
  && pip3 install markupsafe==2.0.0 \
  && pip3 install conan

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

RUN mkdir -p /root/.conan2/profiles
COPY ./default.ini /root/.conan2/profiles/default

RUN echo "rapatas-transmitron-linux-x86-64" > /etc/compilername
