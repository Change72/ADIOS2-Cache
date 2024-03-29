/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * BP4Base.cpp
 *
 *  Created on: Aug 1, 2018
 *  Author: William F Godoy godoywf@ornl.gov
 *          Lipeng Wan wanl@ornl.gov
 *          Norbert Podhorszki pnb@ornl.gov
 */
#include "BP4Base.h"

#include <algorithm> // std::transform
#include <iostream>  //std::cout Warnings

#include "adios2/common/ADIOSTypes.h"     //PathSeparator
#include "adios2/helper/adiosFunctions.h" //CreateDirectory, StringToTimeUnit,

namespace adios2
{
namespace format
{

BP4Base::BP4Base(helper::Comm const &comm) : BPBase(comm) {}

std::vector<std::string>
BP4Base::GetBPBaseNames(const std::vector<std::string> &names) const noexcept
{
    std::vector<std::string> bpBaseNames;
    bpBaseNames.reserve(names.size());

    for (const auto &name : names)
    {
        bpBaseNames.push_back(helper::RemoveTrailingSlash(name));
    }
    return bpBaseNames;
}

std::vector<std::string>
BP4Base::GetBPMetadataFileNames(const std::vector<std::string> &names) const noexcept
{
    std::vector<std::string> metadataFileNames;
    metadataFileNames.reserve(names.size());
    for (const auto &name : names)
    {
        metadataFileNames.push_back(GetBPMetadataFileName(name));
    }
    return metadataFileNames;
}

std::string BP4Base::GetBPMetadataFileName(const std::string &name) const noexcept
{
    const std::string bpName = helper::RemoveTrailingSlash(name);
    const size_t index = 0; // global metadata file is generated by rank 0
    /* the name of the metadata file is "md.0" */
    const std::string bpMetaDataRankName(bpName + PathSeparator + "md." + std::to_string(index));
    return bpMetaDataRankName;
}

std::vector<std::string>
BP4Base::GetBPMetadataIndexFileNames(const std::vector<std::string> &names) const noexcept
{
    std::vector<std::string> metadataIndexFileNames;
    metadataIndexFileNames.reserve(names.size());
    for (const auto &name : names)
    {
        metadataIndexFileNames.push_back(GetBPMetadataIndexFileName(name));
    }
    return metadataIndexFileNames;
}

std::string BP4Base::GetBPMetadataIndexFileName(const std::string &name) const noexcept
{
    const std::string bpName = helper::RemoveTrailingSlash(name);
    /* the name of the metadata index file is "md.idx" */
    const std::string bpMetaDataIndexRankName(bpName + PathSeparator + "md.idx");
    return bpMetaDataIndexRankName;
}

std::vector<std::string>
BP4Base::GetBPVersionFileNames(const std::vector<std::string> &names) const noexcept
{
    std::vector<std::string> versionFileNames;
    versionFileNames.reserve(names.size());
    for (const auto &name : names)
    {
        versionFileNames.push_back(GetBPVersionFileName(name));
    }
    return versionFileNames;
}

std::string BP4Base::GetBPVersionFileName(const std::string &name) const noexcept
{
    const std::string bpName = helper::RemoveTrailingSlash(name);
    /* the name of the version file is ".bpversion" */
    const std::string bpVersionFileName(bpName + PathSeparator + ".bpversion");
    return bpVersionFileName;
}

std::vector<std::string>
BP4Base::GetBPActiveFlagFileNames(const std::vector<std::string> &names) const noexcept
{
    std::vector<std::string> metadataIndexFileNames;
    metadataIndexFileNames.reserve(names.size());
    for (const auto &name : names)
    {
        metadataIndexFileNames.push_back(GetBPActiveFlagFileName(name));
    }
    return metadataIndexFileNames;
}

std::string BP4Base::GetBPActiveFlagFileName(const std::string &name) const noexcept
{
    const std::string bpName = helper::RemoveTrailingSlash(name);
    /* the name of the metadata index file is "md.idx" */
    const std::string bpMetaDataIndexRankName(bpName + PathSeparator + "active");
    return bpMetaDataIndexRankName;
}

std::vector<std::string>
BP4Base::GetBPSubStreamNames(const std::vector<std::string> &names) const noexcept
{
    std::vector<std::string> bpNames;
    bpNames.reserve(names.size());

    for (const auto &name : names)
    {
        bpNames.push_back(GetBPSubStreamName(name, static_cast<unsigned int>(m_RankMPI)));
    }
    return bpNames;
}

std::string BP4Base::GetBPSubFileName(const std::string &name, const size_t subFileIndex,
                                      const bool hasSubFiles, const bool isReader) const noexcept
{
    return GetBPSubStreamName(name, subFileIndex, hasSubFiles, isReader);
}

size_t BP4Base::GetBPIndexSizeInData(const std::string &variableName,
                                     const Dims &count) const noexcept
{
    size_t indexSize = 23; // header
    indexSize += 4 + 32;   // "[VMD" and padded " *VMD]" up to 31 char length
    indexSize += variableName.size();

    // characteristics 3 and 4, check variable number of dimensions
    const size_t dimensions = count.size();
    indexSize += 28 * dimensions; // 28 bytes per dimension
    indexSize += 1;               // id

    // characteristics, offset + payload offset in data
    indexSize += 2 * (1 + 8);
    // characteristic 0, if scalar add value, for now only allowing string
    if (dimensions == 1)
    {
        indexSize += 2 * sizeof(uint64_t); // complex largest size
        indexSize += 1;                    // id
        indexSize += 1;                    // id
    }

    // characteristic statistics
    indexSize += 5;                   // count + length
    if (m_Parameters.StatsLevel == 1) // default, only min and max and dimensions
    {
        const size_t nElems = helper::GetTotalSize(count);
        const size_t nSubblocks = nElems / m_Parameters.StatsBlockSize;
        indexSize += 2 * (nSubblocks + 1) * (2 * sizeof(uint64_t) + 1);
        indexSize += dimensions * 2;
        indexSize += 1 + 2; // id + # of subblocks field
    }
    // dimensions
    indexSize += 28 * dimensions + 1;

    // extra 12 bytes for attributes in case of last variable
    // extra 4 bytes for PGI] in case of last variable
    return indexSize + 12 + 4;
}

// PROTECTED
BP4Base::ElementIndexHeader
BP4Base::ReadElementIndexHeader(const std::vector<char> &buffer, size_t &position,
                                const bool isLittleEndian) const noexcept
{
    ElementIndexHeader header;
    header.Length = helper::ReadValue<uint32_t>(buffer, position, isLittleEndian);
    header.MemberID = helper::ReadValue<uint32_t>(buffer, position, isLittleEndian);
    header.GroupName = ReadBPString(buffer, position, isLittleEndian);
    header.Name = ReadBPString(buffer, position, isLittleEndian);
    // header.Path = ReadBP4String(buffer, position, isLittleEndian);
    header.Path = "";
    header.Order = helper::ReadValue<char>(buffer, position, isLittleEndian);
    // uint8_t unused =
    helper::ReadValue<uint8_t>(buffer, position, isLittleEndian);

    header.DataType = helper::ReadValue<int8_t>(buffer, position, isLittleEndian);
    header.CharacteristicsSetsCount = helper::ReadValue<uint64_t>(buffer, position, isLittleEndian);

    return header;
}

// PRIVATE
std::string BP4Base::GetBPSubStreamName(const std::string &name, const size_t id,
                                        const bool hasSubFiles, const bool isReader) const noexcept
{
    if (!hasSubFiles)
    {
        return name;
    }

    const std::string bpName = helper::RemoveTrailingSlash(name);

    const size_t index = isReader                  ? id
                         : m_Aggregator.m_IsActive ? m_Aggregator.m_SubStreamIndex
                                                   : id;

    /* the name of a data file starts with "data." */
    const std::string bpRankName(bpName + PathSeparator + "data." + std::to_string(index));
    return bpRankName;
}

} // end namespace format
} // end namespace adios2
