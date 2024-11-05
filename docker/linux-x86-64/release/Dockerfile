FROM rapatas-transmitron-linux-x86-64

RUN true \
  && mkdir wxWidgets \
  && cd wxWidgets \
  && wget https://github.com/wxWidgets/wxWidgets/releases/download/v3.2.6/wxWidgets-3.2.6.zip \
  && unzip *.zip \
  && mkdir buildgtk \
  && cd buildgtk \
  && ../configure \
    --with-gtk \
    --disable-mediactrl  \
    --disable-webview \
    --without-libtiff \
    --disable-shared \
    --enable-stl \
  && make -j $(nproc) \
  && make install \
  && cd /workspace \
  && rm -rf *

COPY ./conanfile.py /workspace
RUN true \
  && sed -i '/build_type/s/=.*$/=Release/' /root/.conan2/profiles/default \
  && conan install . --build=missing \
  && rm -rf *

RUN true \
  && apt-get install -y git \
  && git config --global --add safe.directory /workspace

RUN echo "rapatas-transmitron-linux-x86-64-release" > /etc/compilername
COPY ./macro-make  /usr/local/bin
