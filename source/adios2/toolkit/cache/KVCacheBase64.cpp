//
// Created by cguo51 on 1/8/24.
//
#include "adios2/toolkit/cache/KVCacheBase64.h"
//namespace adios2
//{
std::string base64Encode(const std::vector<char> &data)
{
    BIO *bio, *b64;
    BUF_MEM *bptr;

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_write(bio, data.data(), static_cast<int>(data.size()));
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bptr);

    std::string result(bptr->data, bptr->length);

    BIO_free_all(bio);

    return result;
}

std::vector<char> base64Decode(const std::string &encoded)
{
    BIO *bio, *b64;
    std::vector<char> result(encoded.size());

    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_new_mem_buf(encoded.c_str(), static_cast<int>(encoded.size()));
    bio = BIO_push(b64, bio);

    int decodedLength = BIO_read(bio, result.data(), static_cast<int>(result.size()));
    result.resize(decodedLength);

    BIO_free_all(bio);

    return result;
}

template <typename T>
void encodeVector(const std::vector<T> &vec, std::string &encodedString)
{
    std::vector<char> rawData(reinterpret_cast<const char *>(vec.data()),
                              reinterpret_cast<const char *>(vec.data() + vec.size()));
    encodedString = base64Encode(rawData);
}

template <typename T>
void decodeVector(const std::string &str, std::vector<T> &vec)
{
    std::vector<char> rawData = base64Decode(str);

    // Calculate the number of elements based on the total size of rawData
    size_t numElements = rawData.size() / sizeof(T);

    // Construct the result vector using the correct size
    vec(reinterpret_cast<const T *>(rawData.data()),
        reinterpret_cast<const T *>(rawData.data() + numElements * sizeof(T)));
}
//}; // adios2