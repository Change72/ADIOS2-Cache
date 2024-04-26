//
// Created by cguo51 on 7/27/23.
//

#ifndef ADIOS2_KVCACHE_QUERYBOX_H
#define ADIOS2_KVCACHE_QUERYBOX_H

#include <set>
#include "json.hpp"

#include "adios2/common/ADIOSTypes.h"

namespace adios2
{
// QueryBox is a class to represent a query box in a multi-dimensional space
class QueryBox
{
public:
    adios2::Dims start{};
    adios2::Dims count{};

    // constructor
    QueryBox() = default;
    QueryBox(const adios2::Dims &start, const adios2::Dims &count) : start(start), count(count){};
    QueryBox(const std::string &key){
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

        this->start = lf_ExtractDimensions(key, "__start_:_");
        this->count = lf_ExtractDimensions(key, "__count_:_");
    }

    // size
    size_t size() const
    {
        size_t s = 1;
        for (auto &d : count)
        {
            s *= d;
        }
        return s;
    }

    // Serialize QueryBox to a JSON string
    static std::string serializeQueryBox(const QueryBox &box)
    {
        nlohmann::json jsonBox;
        jsonBox["start"] = box.start;
        jsonBox["count"] = box.count;
        return jsonBox.dump();
    }

    // Deserialize JSON string to a QueryBox
    QueryBox deserializeQueryBox(const std::string &jsonString)
    {
        nlohmann::json jsonBox = nlohmann::json::parse(jsonString);
        QueryBox box;
        box.start = jsonBox["start"].get<adios2::Dims>();
        box.count = jsonBox["count"].get<adios2::Dims>();
        return box;
    }

    // determine if a query box is equal to another query box
    bool operator==(const QueryBox &box) const
    {
        return start == box.start && count == box.count;
    }

    // determine if a query box is interacted in another query box, return intersection part as a new query box
    bool isInteracted (const QueryBox &box, QueryBox &intersection) const
    {
        if (start.size() != box.start.size() || start.size() != count.size() ||
            start.size() != box.count.size())
        {
            return false;
        }
        for (size_t i = 0; i < start.size(); ++i)
        {
            if (start[i] > box.start[i] + box.count[i] || box.start[i] > start[i] + count[i])
            {
                return false;
            }
        }
        for (size_t i = 0; i < start.size(); ++i)
        {
            intersection.start[i] = std::max(start[i], box.start[i]);
            intersection.count[i] =
                std::min(start[i] + count[i], box.start[i] + box.count[i]) - intersection.start[i];
        }
        return true;
    }

    // determine if a query box is fully contained in another query box
    bool isFullContainedBy(const QueryBox &box)
    {
        if (start.size() != box.start.size() || start.size() != count.size() ||
            start.size() != box.count.size())
        {
            return false;
        }
        for (size_t i = 0; i < start.size(); ++i)
        {
            if (start[i] < box.start[i] || start[i] + count[i] > box.start[i] + box.count[i])
            {
                return false;
            }
        }
        return true;
    }

    // cut a query box from another interaction box, return a list of regular box
    // remainingBox is the big one, this is small one
    void interactionCut(const QueryBox &remainingBox, std::vector<QueryBox> &regularBoxes)
    {
        if (remainingBox == *this)
        {
            return;
        }

        // find the max cut dimension
        size_t maxCutDimSize = 0;
        QueryBox maxCutDimBox;
        for (size_t i = 0; i < start.size(); ++i)
        {
            if (start[i] == remainingBox.start[i] && count[i] == remainingBox.count[i])
            {
                continue;
            }
            else {
                if (start[i] != remainingBox.start[i]){
                    size_t cutDimDiff = start[i] - remainingBox.start[i];
                    size_t cutDimSize = remainingBox.size() / remainingBox.count[i] * cutDimDiff;
                    if (cutDimSize > maxCutDimSize)
                    {
                        maxCutDimSize = cutDimSize;
                        maxCutDimBox = QueryBox(remainingBox.start, remainingBox.count);
                        maxCutDimBox.count[i] = cutDimDiff;
                    }
                }

                if (start[i] + count[i] != remainingBox.start[i] + remainingBox.count[i]){
                    size_t cutDimDiff = remainingBox.start[i] + remainingBox.count[i] - start[i] - count[i];
                    size_t cutDimSize = remainingBox.size() / count[i] * cutDimDiff;
                    if (cutDimSize > maxCutDimSize)
                    {
                        maxCutDimSize = cutDimSize;
                        maxCutDimBox = QueryBox(remainingBox.start, remainingBox.count);
                        maxCutDimBox.start[i] = start[i] + count[i];
                        maxCutDimBox.count[i] = cutDimDiff;
                    }
                }
            }
        }

        // cut the max cut dimension
        if (maxCutDimSize > 0)
        {
            regularBoxes.push_back(maxCutDimBox);
            QueryBox remainingBox1 = QueryBox(remainingBox.start, remainingBox.count);
            for (size_t i = 0; i < remainingBox.start.size(); ++i)
            {
                if (maxCutDimBox.start[i] == remainingBox.start[i] && maxCutDimBox.count[i] == remainingBox.count[i])
                {
                    continue;
                }
                else {
                    if (maxCutDimBox.start[i] != remainingBox.start[i])
                    {
                        remainingBox1.count[i] = maxCutDimBox.start[i] - remainingBox.start[i];
                    } else {
                        remainingBox1.start[i] = maxCutDimBox.start[i] + maxCutDimBox.count[i];
                        remainingBox1.count[i] = remainingBox.start[i] + remainingBox.count[i] - remainingBox1.start[i];
                    }

                }
            }
            interactionCut(remainingBox1, regularBoxes);
        }
    }
    
};
};
#endif // UNITTEST_QUERYBOX_H
