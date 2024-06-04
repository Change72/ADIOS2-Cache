//
// Created by cguo51 on 12/30/23.
//
#ifndef KVCACHECOMMON_TCC
#define KVCACHECOMMON_TCC

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
        std::cout << "------------------------------------------------" << std::endl;
        std::cout << "Connected to kvcache server. KV Cache Version Control: V1.1" << std::endl;
    }
}

void KVCacheCommon::closeConnection()
{
    redisFree(m_redisContext);
}

template <typename T>
void KVCacheCommon::set(std::string key, const std::vector<T>& vec)
{
    m_key = key;
    encodeVector(vec, m_value);
//    m_key = "test";
//    m_value = "test";
    m_command = "SET " + m_key + " " + m_value;
    m_redisReply = (redisReply *)redisCommand(m_redisContext, m_command.c_str());
    if (m_redisReply == NULL)
    {
        std::cout << "Error: " << m_redisContext->errstr << std::endl;
    }
    else
    {
        std::cout << "SET Key: " << m_key << " Value size: " << vec.size() << std::endl;
        freeReplyObject(m_redisReply);
    }
}

template <typename T>
void KVCacheCommon::get(std::string key, std::vector<T>& vec)
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
        // std::cout << "GET: " << m_redisReply->str << std::endl;
        decodeVector(m_redisReply->str, vec);
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
    std::cout << "Try to find the key: IF EXISTS: " << m_command.c_str() << std::endl;
    // m_command = "EXISTS mytest";
    m_redisReply = (redisReply *)redisCommand(m_redisContext, m_command.c_str());
    if (m_redisReply == NULL)
    {
        std::cout << "Key does not exist" << m_command.c_str() << std::endl;
        return false;
    }
    else
    {
        if (!m_redisReply->integer)
        {
            std::cout << "Key does not exist" << m_command.c_str()  << std::endl;
            return false;
        }
        // std::cout << "EXISTS: " << m_redisReply->str << std::endl;
        freeReplyObject(m_redisReply);
        return true;
    }
}

std::string KVCacheCommon::keyPrefix(char *VarName, size_t AbsStep, size_t BlockID)
{
    return VarName + std::to_string(AbsStep) + std::to_string(BlockID);
}

std::string KVCacheCommon::keyComposition(const std::string &key_prefix, Dims Start, Dims Count)
{
    std::string box = QueryBox::serializeQueryBox(QueryBox{Start, Count});
    std::string cacheKey = key_prefix + box;
    // replace special characters
    std::replace(cacheKey.begin(), cacheKey.end(), '"', '_');
    std::replace(cacheKey.begin(), cacheKey.end(), ',', '_');
    std::replace(cacheKey.begin(), cacheKey.end(), '(', '_');
    std::replace(cacheKey.begin(), cacheKey.end(), ')', '_');
    std::replace(cacheKey.begin(), cacheKey.end(), '[', '_');
    std::replace(cacheKey.begin(), cacheKey.end(), ']', '_');
    std::replace(cacheKey.begin(), cacheKey.end(), '{', '_');
    std::replace(cacheKey.begin(), cacheKey.end(), '}', '_');
    return cacheKey;
}

void KVCacheCommon::keyPrefixExistence(const std::string &key_prefix, std::set<std::string> &keys)
{
    std::string keyPattern = key_prefix + "*";
    m_command = "KEYS " + keyPattern;
    m_redisReply = (redisReply *)redisCommand(m_redisContext, m_command.c_str());
    if (m_redisReply == NULL)
    {
        std::cout << "Error: " << m_redisContext->errstr << std::endl;
    }
    else
    {
        for (int i = 0; i < m_redisReply->elements; i++)
        {
            keys.insert(m_redisReply->element[i]->str);
        }
        freeReplyObject(m_redisReply);
    }
}

template <typename T>
void KVCacheCommon::encodeVector(const std::vector<T>& vec, std::string& encodedString) {
    size_t vecSize = vec.size() * sizeof(T);
    const unsigned char* vecBytes = reinterpret_cast<const unsigned char*>(vec.data());

    size_t encodedSize = vecSize * 3 / 2;
    std::vector<unsigned char> encodedBytes(encodedSize);

    size_t sizeAfterEncoded = adios2sysBase64_Encode(vecBytes, vecSize, encodedBytes.data(), 0);

    // Resize the vector to the actual size
    encodedBytes.resize(sizeAfterEncoded);

    // Convert the encoded bytes to a string
    encodedString.assign(encodedBytes.begin(), encodedBytes.end());
}

template <typename T>
void KVCacheCommon::decodeVector(const std::string& str, std::vector<T>& vec) {
    size_t decodedSize = str.size() * 2;
    std::vector<unsigned char> decodedBytes(decodedSize);

    size_t sizeAfterDecoded = adios2sysBase64_Decode(reinterpret_cast<const unsigned char*>(str.data()), str.size(), decodedBytes.data(), decodedSize);

    // Resize the vector to the actual size
    decodedBytes.resize(sizeAfterDecoded);

    // Copy the decoded bytes to the vector
    vec.resize(sizeAfterDecoded / sizeof(T));
    memcpy(vec.data(), decodedBytes.data(), sizeAfterDecoded);

}

}; // namespace adios2
#endif // KVCACHECOMMON_TCC