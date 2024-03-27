//
// Created by cguo51 on 12/30/23.
//
#ifndef KVCACHECOMMON_INL
#define KVCACHECOMMON_INL
#include "adios2/toolkit/cache/KVCacheBase64.h"

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
        std::cout << "SET: " << m_redisReply->str << std::endl;
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
    std::cout << "EXISTS: " << m_command.c_str() << std::endl;
    // m_command = "EXISTS mytest";
    m_redisReply = (redisReply *)redisCommand(m_redisContext, m_command.c_str());
    if (m_redisReply == NULL)
    {
        std::cout << "Key does not exist" << std::endl;
        return false;
    }
    else
    {
        if (!m_redisReply->integer)
        {
            std::cout << "Key does not exist" << std::endl;
            return false;
        }
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
    // replace special characters
    std::replace(cacheKey.begin(), cacheKey.end(), '"', '_');
    std::replace(cacheKey.begin(), cacheKey.end(), ',', '_');
    std::replace(cacheKey.begin(), cacheKey.end(), '(', '_');
    std::replace(cacheKey.begin(), cacheKey.end(), ')', '_');
    std::replace(cacheKey.begin(), cacheKey.end(), '[', '_');
    std::replace(cacheKey.begin(), cacheKey.end(), ']', '_');
    std::replace(cacheKey.begin(), cacheKey.end(), '{', '_');
    std::replace(cacheKey.begin(), cacheKey.end(), '}', '_');
}

void KVCacheCommon::keyPrefixExistence(char *VarName, size_t AbsStep, size_t BlockID, std::set<std::string> &keys)
{
    std::string key = VarName + std::to_string(AbsStep) + std::to_string(BlockID);
    std::string keyPattern = key + "*";
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

void KVCacheCommon::extractStartCount(const std::string &key, Dims &Start, Dims &Count)
{
    // sample key: "U3218446744073709551615__count_:_64_64_64___start_:_0_0_0__", count [64, 64, 64], start [0, 0, 0]
    // using Dims = std::vector<size_t>;
    auto lf_ExtractDimensions = [](const std::string &key, const std::string &delimiter) -> Dims {
        size_t pos = key.find(delimiter);
        size_t end = key.find("__", pos + delimiter.length());
        std::string dimStr = key.substr(pos + delimiter.length(), end - pos - delimiter.length());
        Dims dimensions;
        std::istringstream dimStream(dimStr);
        std::string token;
        while (std::getline(dimStream, token, '_')) {
            dimensions.push_back(std::stoul(token));
        }
        return dimensions;
    };

    Start = lf_ExtractDimensions(key, "__start_:_");
    Count = lf_ExtractDimensions(key, "__count_:_");
}

std::string KVCacheCommon::base64Encode(const std::vector<char> &data)
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

std::vector<char> KVCacheCommon::base64Decode(const std::string &encoded)
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
void KVCacheCommon::encodeVector(const std::vector<T> &vec, std::string &encodedString)
{
    std::vector<char> rawData(reinterpret_cast<const char *>(vec.data()),
                              reinterpret_cast<const char *>(vec.data() + vec.size()));
    encodedString = base64Encode(rawData);
}

template <typename T>
void KVCacheCommon::decodeVector(const std::string &str, std::vector<T> &vec)
{
    std::vector<char> rawData = base64Decode(str);

    // Calculate the number of elements based on the total size of rawData
    size_t numElements = rawData.size() / sizeof(T);

    // Construct the result vector using the correct size
    std::vector<T> result(reinterpret_cast<const T *>(rawData.data()),
        reinterpret_cast<const T *>(rawData.data() + numElements * sizeof(T)));
    vec = result;
}

}; // namespace adios2
#endif // KVCACHECOMMON_INL