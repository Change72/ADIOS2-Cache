#!/bin/bash

if [ -z ${BUILD_DIR} ]
then
    BUILD_DIR=${PWD}/build-cache
fi

if [ ! -d ${BUILD_DIR} ]
then
    mkdir -p ${BUILD_DIR}
fi

cd ${BUILD_DIR}

redis_dir=${BUILD_DIR}/redis
if [ ! -d ${redis_dir} ]
then
    git clone https://github.com/redis/redis.git ${redis_dir}
    cd ${redis_dir}
    git checkout tags/7.2.3
    make
    nohup ./src/redis-server &
    cd -
fi

openssl_dir=${BUILD_DIR}/openssl
if [ ! -d ${openssl_dir} ]
then
    git clone https://github.com/openssl/openssl.git ${openssl_dir}
    cd ${openssl_dir}
    git checkout tags/openssl-3.2.0
    ./Configure 
    make -j32
    cd -
fi

cmake .. -DADIOS2_USE_Python=ON \
        -DHIREDIS_INCLUDE_DIR=${redis_dir}/deps/ \
        -DHIREDIS_LIBRARY=${redis_dir}/deps/hiredis/libhiredis.a \
        -DOPENSSL_INCLUDE_DIR=${openssl_dir}/include \
        -DOPENSSL_CRYPTO_LIBRARY=${openssl_dir}/libcrypto.a \
        -DOPENSSL_SSL_LIBRARY=${openssl_dir}/libssl.a

make -j32

nohup ./bin/remote_server > remote_server.log 2>&1 &

export DoRemote=1
export useKVCache=1
