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
    nsis

RUN true \
  && apt-get install -y python3 python3-pip \
  && pip3 install conan==1.62.0 \
  && conan profile new --force default --detect

RUN mkdir -p /root/.conan/profiles
COPY ./conan.profile /root/.conan/profiles/windows

RUN true \
  && apt-get update \
  && apt-get install -y \
    curl \
    wget \
    vim \
    tree \
    unzip

RUN echo "rapatas-transmitron-windows-x86-64" > /etc/compilername
