//
// Created by cguo51 on 12/30/23.
//

#include "KVCacheCommon.h"

namespace adios2
{

void KVCacheCommon::openConnection()
{
    m_redisContext = redisConnect(m_host.c_str(), m_port);
    if (m_redisContext == NULL || m_redisContext->err)
    {
        if (m_redisContext)
        {
            std::cout << "Error: " << m_redisContext->errstr << std::endl;
            redisFree(m_redisContext);
        }
        else
        {
            std::cout << "Can't allocate kvcache context" << std::endl;
        }
    }
    else
    {
        std::cout << "Connected to kvcache server" << std::endl;
    }
}

void KVCacheCommon::closeConnection()
{
    redisFree(m_redisContext);
}

void KVCacheCommon::set(std::string key, void* Data)
{
    m_key = key;
    m_value = (char*)Data;
    m_command = "SET " + m_key + " " + m_value;
    m_redisReply = (redisReply *)redisCommand(m_redisContext, m_command.c_str());
    if (m_redisReply == NULL)
    {
        std::cout << "Error: " << m_redisContext->errstr << std::endl;
    }
    else
    {
        std::cout << "SET: " << m_redisReply->str << std::endl;
        freeReplyObject(m_redisReply);
    }
}

void KVCacheCommon::get(std::string key, void* Data)
{
    m_key = key;
    m_command = "GET " + m_key;
    m_redisReply = (redisReply *)redisCommand(m_redisContext, m_command.c_str());
    if (m_redisReply == NULL)
    {
        std::cout << "Error: " << m_redisContext->errstr << std::endl;
    }
    else
    {
        std::cout << "GET: " << m_redisReply->str << std::endl;
        Data = (void*)m_redisReply->str;
        freeReplyObject(m_redisReply);
    }
}

void KVCacheCommon::del(std::string key)
{
    m_key = key;
    m_command = "DEL " + m_key;
    m_redisReply = (redisReply *)redisCommand(m_redisContext, m_command.c_str());
    if (m_redisReply == NULL)
    {
        std::cout << "Error: " << m_redisContext->errstr << std::endl;
    }
    else
    {
        std::cout << "DEL: " << m_redisReply->str << std::endl;
        freeReplyObject(m_redisReply);
    }
}

bool KVCacheCommon::exists(std::string key)
{
    m_key = key;
    m_command = "EXISTS " + m_key;
    m_redisReply = (redisReply *)redisCommand(m_redisContext, m_command.c_str());
    if (m_redisReply == NULL)
    {
        std::cout << "Key does not exist" << std::endl;
        return false;
    }
    else
    {
        std::cout << "EXISTS: " << m_redisReply->str << std::endl;
        freeReplyObject(m_redisReply);
        return true;
    }
}

void KVCacheCommon::keyComposition(char *VarName, size_t AbsStep, size_t BlockID, Dims Start, Dims Count, std::string &cacheKey)
{
    std::string key = VarName + std::to_string(AbsStep) + std::to_string(BlockID);
    std::string box = QueryBox::serializeQueryBox(QueryBox{Start, Count});
    cacheKey = key + box;
}



}; // namespace adios2
