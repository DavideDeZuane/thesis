FROM ubuntu:latest

LABEL mantainer="Davide De Zuane"
LABEL description="Questo Dockerfile compila e installa OpenSSL, liboqs e oqs-provider."

VOLUME /workspace

ENV WORKSPACE=/root/quantumsafe
ENV BUILD_DIR=$WORKSPACE/build
ENV OPENSSL_CONF=$BUILD_DIR/ssl/openssl.cnf
ENV OPENSSL_MODULES=$BUILD_DIR/lib

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    git \
    build-essential \
    perl \
    cmake \
    autoconf \
    libtool \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*


RUN mkdir -p $BUILD_DIR/lib64 && ln -s $BUILD_DIR/lib64 $BUILD_DIR/lib

WORKDIR $WORKSPACE

RUN git clone https://github.com/openssl/openssl.git $WORKSPACE/openssl
RUN cd $WORKSPACE/openssl && \
    ./Configure \
        --prefix=$BUILD_DIR \
        no-ssl no-tls1 no-tls1_1 no-afalgeng \
        no-shared threads -lm && \
    make -j $(nproc) && \
    make -j $(nproc) install_sw install_ssldirs

# Liboqs
RUN git clone https://github.com/open-quantum-safe/liboqs.git $WORKSPACE/liboqs
RUN cd $WORKSPACE/liboqs && mkdir build && cd build && cmake \
        -DCMAKE_INSTALL_PREFIX=$BUILD_DIR \
        -DBUILD_SHARED_LIBS=ON \
        -DOQS_USE_OPENSSL=OFF \
        -DCMAKE_BUILD_TYPE=Release \
        -DOQS_BUILD_ONLY_LIB=ON \
        -DOQS_DIST_BUILD=ON \
        .. && \
    make -j $(nproc) && \
    make -j $(nproc) install

# OQS Provider
RUN git clone https://github.com/open-quantum-safe/oqs-provider.git $WORKSPACE/oqs-provider
RUN cd $WORKSPACE/oqs-provider && liboqs_DIR=$BUILD_DIR cmake \
        -DCMAKE_INSTALL_PREFIX=$WORKSPACE/oqs-provider \
        -DOPENSSL_ROOT_DIR=$BUILD_DIR \
        -DCMAKE_BUILD_TYPE=Release \
        -S . \
        -B _build && \
    cmake --build _build && \
    cp _build/lib/* $BUILD_DIR/lib/

# Edit OpenSSL config to use oqsprovider
RUN sed -i "s/default = default_sect/default = default_sect\noqsprovider = oqsprovider_sect/g" $BUILD_DIR/ssl/openssl.cnf && \
    sed -i "s/\[default_sect\]/\[default_sect\]\nactivate = 1\n\[oqsprovider_sect\]\nactivate = 1\n/g" $BUILD_DIR/ssl/openssl.cnf

RUN $BUILD_DIR/bin/openssl list -providers -verbose -provider oqsprovider

# Other tool
RUN apt-get update && apt-get install -y bc
RUN apt-get clean

COPY script/cert /usr/local/bin/cert
RUN chomd +x /usr/local/bin/cert


# Default command
CMD ["/bin/bash"]

