//
// Created by cguo51 on 1/8/24.
//
#ifndef ADIOS2_KVCACHEBASE64_H
#define ADIOS2_KVCACHEBASE64_H
#include <iostream>
#include <vector>
#include <string>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
//namespace adios2
//{

std::string base64Encode(const std::vector<char>& data);

std::vector<char> base64Decode(const std::string& encoded);

template <typename T>
void encodeVector(const std::vector<T>& vec, std::string& encodedString);

template <typename T>
void decodeVector(const std::string& str, std::vector<T>& vec);
//}; // adios2
#endif // ADIOS2_KVCACHEBASE64_H
