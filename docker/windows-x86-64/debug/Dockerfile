FROM rapatas-transmitron-windows-x86-64

COPY ./conanfile.py  /workspace
RUN true \
  && sed -i '/build_type/s/=.*$/=Debug/' /root/.conan2/profiles/windows \
  && conan install /workspace --build=missing --profile:build=default --profile:host=windows

RUN true \
  && mkdir wxWidgets \
  && cd wxWidgets \
  && wget https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.6/wxWidgets-3.2.6.zip \
  && unzip *.zip \
  && mkdir buildwin \
  && cd buildwin \
  && ../configure \
    --prefix=/usr/x86_64-w64-mingw32 \
    --host=x86_64-w64-mingw32 \
    --build=x86_64-linux \
    --disable-mediactrl  \
    --disable-webview \
    --without-libtiff \
    --disable-shared \
    --enable-stl \
    --enable-debug \
  && make -j $(nproc) \
  && make install \
  && cd /workspace \
  && rm -rf *

RUN true \
  && apt-get install -y git \
  && git config --global --add safe.directory /workspace

RUN echo "rapatas-transmitron-windows-x86-64-debug" > /etc/compilername
COPY ./macro-make  /usr/local/bin
