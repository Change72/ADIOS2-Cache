//
// Created by chang on 3/20/25.
//

#ifndef ADIOS2_KVCACHEMETADATA_H
#define ADIOS2_KVCACHEMETADATA_H

#include "QueryBox.h"

#ifdef ADIOS2_HAVE_SpatialIndex
#include <spatialindex/SpatialIndex.h>
#endif

namespace adios2
{

namespace kvcache
{

#ifdef ADIOS2_HAVE_SpatialIndex
class MyVisitor : public IVisitor
{
public:
    MyVisitor() = default;

    vector<const IShape*> results;  // Store overlapping regions

    void visitNode(const INode& n) override{
        // Do nothing
    }

    // Called when visiting data (leaf entries)
    void visitData(const IData& d) override {
        IShape* shape = nullptr;
        d.getShape(&shape);  // Get the region (shape) of the data entry
        results.push_back(shape);  // Store overlapping region
    }

    void visitData(std::vector<const IData*>& v) override {
        for (const IData* data : v) {
            visitData(*data);
        }
    }
};
#endif

class KVCacheMetadata
{

#ifdef ADIOS2_HAVE_SpatialIndex
private:
    size_t m_dim;

public:
    SpatialIndex::ISpatialIndex* m_tree = nullptr;

    KVCacheMetadata(size_t dim) : m_dim(dim) {
        m_tree = SpatialIndex::RTree::createNewRTree(SpatialIndex::StorageManager::createNewMemoryStorageManager(), 0.7, 20, 20, dim, SpatialIndex::RTree::RV_RSTAR);
    }

    ~KVCacheMetadata() {
        delete m_tree;
    }

    void Insert(const QueryBox &queryBox) {
        double* pLow = new double[m_dim];
        double* pHigh = new double[m_dim];

        for (size_t i = 0; i < m_dim; ++i) {
            pLow[i] = queryBox.Start[i];
            pHigh[i] = queryBox.Start[i] + queryBox.Count[i];
        }

        SpatialIndex::Region r(pLow, pHigh, m_dim);
        m_tree->insertData(0, nullptr, r);
    }

    void Query(const QueryBox &queryBox, QueryBox &maxOverlapBox) {
        double* pLow = new double[m_dim];
        double* pHigh = new double[m_dim];

        for (size_t i = 0; i < m_dim; ++i) {
            pLow[i] = queryBox.Start[i];
            pHigh[i] = queryBox.Start[i] + queryBox.Count[i];
        }

        MyVisitor visitor;
        SpatialIndex::Region r(pLow, pHigh, m_dim);
        m_tree->intersectsWithQuery(r, visitor);

        size_t max_overlap_size = 0;
        for (const IShape* shape : visitor.results) {
            const SpatialIndex::Region* region = dynamic_cast<const SpatialIndex::Region*>(shape);
            QueryBox overlapBox = QueryBox(m_dim);
            for (size_t i = 0; i < m_dim; ++i) {
                    overlapBox.Start[i] = region->m_pLow[i];
                    overlapBox.Count[i] = region->m_pHigh[i] - region->m_pLow[i];
            }
            QueryBox intersectionBox = QueryBox(m_dim);
            overlapBox.IsInteracted(queryBox, intersectionBox);

            size_t overlap_size = overlapBox.size();
            if (overlap_size > max_overlap_size) {
                    max_overlap_size = overlap_size;
                    maxOverlapBox = overlapBox;
            }
        }
    }

#else
public:
    KVCacheMetadata(size_t dim) {}
    ~KVCacheMetadata() {}

    void Insert(const QueryBox &queryBox) {}
    void Query(const QueryBox &queryBox, QueryBox &maxOverlapBox) {}
#endif

};
}; // namespace kvcache
}; // adios2
#endif // ADIOS2_KVCACHEMETADATA_H
