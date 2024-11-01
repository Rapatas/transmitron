FROM ubuntu:24.04

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

ENV PATH="${PATH}:/root/.local/bin"
RUN true \
  && apt-get install -y python3 pipx \
  && pipx ensurepath \
  && pipx install conan

RUN true \
  && apt-get update \
  && apt-get install -y \
    curl \
    wget \
    vim \
    tree \
    unzip

RUN mkdir -p /root/.conan2/profiles
COPY ./default.ini /root/.conan2/profiles/default
COPY ./windows.ini /root/.conan2/profiles/windows

RUN echo "rapatas-transmitron-windows-x86-64" > /etc/compilername
