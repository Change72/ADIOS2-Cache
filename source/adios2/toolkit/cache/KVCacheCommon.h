//
// Created by cguo51 on 12/30/23.
//

#ifndef ADIOS2_KVCACHECOMMON_H
#define ADIOS2_KVCACHECOMMON_H
#include <hiredis/hiredis.h>
#include "adios2/toolkit/cache/QueryBox.h"
#include <iostream>


// namespace adios2::KVCache

namespace adios2
{

class KVCacheCommon
{
public:
    std::string m_host;
    int m_port;
    redisContext *m_redisContext;
    redisReply *m_redisReply;
    std::string m_key;
    std::string m_value;
    std::string m_command;

    KVCacheCommon(std::string host="localhost", int port=6379): m_host(host), m_port(port){};

    inline void openConnection();

    inline void closeConnection();

    template <typename T>
    void set(std::string key, const std::vector<T>& vec);

    template <typename T>
    void get(std::string key, std::vector<T>& vec);

    inline void del(std::string key);

    inline bool exists(std::string key);

    inline void keyComposition(char *VarName, size_t AbsStep, size_t BlockID, Dims Start, Dims Count, std::string &cacheKey);

    inline void keyPrefixExistence(char *VarName, size_t AbsStep, size_t BlockID, std::set<std::string> &keys);

    inline void extractStartCount(const std::string &key, Dims &Start, Dims &Count);

//    template <typename T>
//    void serializeVector(const std::vector<T>& vec, std::string& serializedString) {
//        nlohmann::json j = vec;
//        serializedString = j.dump();
//    }
//
//    template <typename T>
//    void deserializeVector(const std::string& str, std::vector<T>& vec) {
//        nlohmann::json j = nlohmann::json::parse(str);
//        vec = j.get<std::vector<T>>();
//    }

    size_t size(Dims Count) const
    {
        size_t size = 1;
        for(auto i: Count)
        {
            size *= i;
        }
        return size;
    }

    inline std::string base64Encode(const std::vector<char>& data);

    inline std::vector<char> base64Decode(const std::string& encoded);

    template <typename T>
    void encodeVector(const std::vector<T>& vec, std::string& encodedString);

    template <typename T>
    void decodeVector(const std::string& str, std::vector<T>& vec);
};


}; // adios2

#include "KVCacheCommon.inl"

#endif // ADIOS2_KVCACHECOMMON_H
