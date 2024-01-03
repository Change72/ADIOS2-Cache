//
// Created by cguo51 on 12/30/23.
//

#ifndef ADIOS2_KVCACHECOMMON_H
#include <hiredis/hiredis.h>
#include "adios2/toolkit/cache/QueryBox.h"
#define ADIOS2_KVCACHECOMMON_H
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

    void openConnection();

    void closeConnection();

    void set(std::string key, void* Data);

    void get(std::string key, void* Data);

    void del(std::string key);

    bool exists(std::string key);

    void keyComposition(char *VarName, size_t AbsStep, size_t BlockID, Dims Start, Dims Count, std::string &cacheKey);
};


}; // adios2

#endif // ADIOS2_KVCACHECOMMON_H
