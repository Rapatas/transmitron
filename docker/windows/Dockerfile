FROM ubuntu:21.04

USER root
WORKDIR /workspace
ENV DEBIAN_FRONTEND=noninteractive

COPY ./windows /root/.conan/profiles/windows

RUN true \
  && apt-get update \
  && apt-get install -y \
    mingw-w64 \
    python3 \
    git \
    python3-pip \
    cmake \
    nsis \
  && echo "rapatas_transmitron_windows_compiler" > /etc/compilername \
  && pip3 install conan \
  && conan remote add bincrafters https://bincrafters.jfrog.io/artifactory/api/conan/public-conan \
  && conan config set general.revisions_enabled=1 \
  && conan install wxwidgets/3.1.5@bincrafters/stable --profile=windows --build=missing -o wxwidgets:webview=False \
  && conan install paho-mqtt-cpp/1.2.0@               --profile=windows --build=missing \
  && conan install nlohmann_json/3.9.1@               --profile=windows --build=missing \
  && conan install tinyxml2/8.0.0@                    --profile=windows --build=missing \
  && conan install fmt/8.0.1@                         --profile=windows --build=missing \
  && conan install spdlog/1.9.2@                      --profile=windows --build=missing \
  && conan install cli11/2.1.1@                       --profile=windows --build=missing

COPY ./toolchain.cmake /opt/cmake-toolchains/linux-x86_64-to-windows-x86_64.cmake
COPY ./macro-make  /usr/local/bin
