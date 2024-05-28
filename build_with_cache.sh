#!/bin/bash

# This script is to build the remote server with cache enabled.
# Attention: hiredis cannot be installed by apt-get, since the default version is too old (libhiredis0.14 (= 0.14.1-2)).
#            We need to build it from source code. You can also use the following scripts to install hiredis (v1.2.0).

if [ -z ${BUILD_DIR} ]
then
    BUILD_DIR=${PWD}/build-cache
fi

if [ ! -d ${BUILD_DIR} ]
then
    mkdir -p ${BUILD_DIR}
fi

cd ${BUILD_DIR}

SW_DIR=${BUILD_DIR}/sw

# redis - in-memory data structure store
redis_dir=${BUILD_DIR}/redis
if [ ! -d ${redis_dir} ]
then
    git clone https://github.com/redis/redis.git ${redis_dir}
    cd ${redis_dir}
    git checkout tags/7.2.3
    make
    # start redis server
    nohup ./src/redis-server & 

    # hiredis - C client library to connect Redis server
    cd ${redis_dir}/deps/hiredis
    mkdir build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX=${SW_DIR}/hiredis
    make -j32
    make install
fi

cd ${BUILD_DIR}

# openssl - TLS/SSL and crypto library
openssl_dir=${BUILD_DIR}/openssl
if [ ! -d ${openssl_dir} ]
then
    git clone https://github.com/openssl/openssl.git ${openssl_dir}
    cd ${openssl_dir}
    git checkout tags/openssl-3.2.0
    ./Configure --prefix=${SW_DIR}/openssl
    make -j32
    make install
    cd -
fi

echo "${SW_DIR}/hiredis;${SW_DIR}/openssl"

cmake .. -DADIOS2_USE_Cache=ON \
        -DCMAKE_PREFIX_PATH=${SW_DIR} \
        -DOPENSSL_CRYPTO_LIBRARY="${SW_DIR}/openssl/lib64/libcrypto.so" \
        -DOPENSSL_SSL_LIBRARY="${SW_DIR}/openssl/lib64/libssl.so"

make -j32

nohup ./bin/remote_server > remote_server.log 2>&1 &

export DoRemote=1
export useKVCache=1
