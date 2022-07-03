FROM conanio/gcc10

WORKDIR /workspace

USER root

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
    libsecret-1-dev \
  && echo "rapatas_transmitron_linux_compiler" > /etc/compilername \
  && conan profile new --force default --detect \
  && conan remote add bincrafters https://bincrafters.jfrog.io/artifactory/api/conan/public-conan \
  && conan config set general.revisions_enabled=1 \
  && sed -i 's/\(=libstdc++\)$/\111/g' /root/.conan/profiles/default \
  && conan install paho-mqtt-cpp/1.2.0@               --build=missing \
  && conan install nlohmann_json/3.9.1@               --build=missing \
  && conan install tinyxml2/8.0.0@                    --build=missing \
  && conan install fmt/8.0.1@                         --build=missing \
  && conan install spdlog/1.9.2@                      --build=missing \
  && conan install cli11/2.1.2@                       --build=missing \
  && conan install date/3.0.1@                        --build=missing \
  && conan install wxwidgets/3.1.5@bincrafters/stable --build=missing -o wxwidgets:webview=False

COPY ./macro-make  /usr/local/bin
