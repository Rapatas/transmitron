FROM rapatas-transmitron-windows-x86-64

RUN true \
  && mkdir wxWidgets \
  && cd wxWidgets \
  && wget https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.4/wxWidgets-3.2.4.zip \
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
  && make -j $(nproc) \
  && make install \
  && cd /workspace \
  && rm -rf *

COPY ./conanfile.py  /workspace
RUN true \
  && conan profile update settings.build_type=Release windows \
  && conan install /workspace --build=missing --profile:build=default --profile:host=windows

RUN echo "rapatas-transmitron-windows-x86-64-release" > /etc/compilername
COPY ./macro-make  /usr/local/bin
