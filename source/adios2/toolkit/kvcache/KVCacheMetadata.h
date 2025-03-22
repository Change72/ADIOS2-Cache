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
public:
    SpatialIndex::ISpatialIndex* m_tree = nullptr;
    size_t m_dim;

    KVCacheMetadata() = default;

    ~KVCacheMetadata() {
        if (m_tree != nullptr) {
            std::cout << *m_tree << std::endl;
            delete m_tree;
        }
    }

    void CreateNewTree(size_t capacity) {
        m_tree = SpatialIndex::RTree::createNewRTree(SpatialIndex::StorageManager::createNewMemoryStorageManager(), 0.7, capacity, capacity, m_dim, SpatialIndex::RTree::RV_RSTAR);
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

    void Query(const QueryBox &queryBox, const size_t &max_depth, size_t current_depth,
               std::vector<QueryBox> &regularBoxes, std::vector<QueryBox> &cachedBoxes) {
        if (current_depth > max_depth)
        {
            return;
        }
        current_depth++;

        double* pLow = new double[m_dim];
        double* pHigh = new double[m_dim];

        for (size_t i = 0; i < m_dim; ++i) {
            pLow[i] = queryBox.Start[i];
            pHigh[i] = queryBox.Start[i] + queryBox.Count[i];
        }

        MyVisitor visitor;
        SpatialIndex::Region r(pLow, pHigh, m_dim);
        m_tree->intersectsWithQuery(r, visitor);

        QueryBox maxOverlapBox = QueryBox(m_dim);
        QueryBox maxInteractBox = QueryBox(m_dim);
        for (const IShape* shape : visitor.results) {
            const SpatialIndex::Region* region = dynamic_cast<const SpatialIndex::Region*>(shape);
            QueryBox overlapBox = QueryBox(m_dim);
            for (size_t i = 0; i < m_dim; ++i) {
                overlapBox.Start[i] = region->m_pLow[i];
                overlapBox.Count[i] = region->m_pHigh[i] - region->m_pLow[i];
            }
            QueryBox intersectionBox = QueryBox(m_dim);
            overlapBox.IsInteracted(queryBox, intersectionBox);

            if (maxInteractBox.size() < intersectionBox.size()) {
                maxOverlapBox = overlapBox;
                maxInteractBox = intersectionBox;
                if (maxInteractBox.size() == queryBox.size()) {
                    break;
                }
            }
        }

        if (maxInteractBox.size() == 0) {
            regularBoxes.push_back(queryBox);
            return;
        }

        cachedBoxes.push_back(maxOverlapBox);

        if (maxInteractBox.size() == queryBox.size()) {
            return;
        }

        if (current_depth == max_depth) {
            maxInteractBox.NdCut(queryBox, regularBoxes);
        } else {
            std::vector<QueryBox> nextBoxes;
            maxInteractBox.NdCut(queryBox, nextBoxes);
            for (QueryBox &nextBox : nextBoxes) {
                this->Query(nextBox, max_depth, current_depth, regularBoxes, cachedBoxes);
            }
        }
    }

#else
public:
    void* m_tree = nullptr;
    size_t m_dim = 0;
    KVCacheMetadata() = default;
    ~KVCacheMetadata() {}

    void CreateNewTree(size_t capacity) {}
    void Insert(const QueryBox &queryBox) {}
    void Query(const QueryBox &queryBox, const size_t &max_depth, size_t current_depth,
               std::vector<QueryBox> &regularBoxes, std::vector<QueryBox> &cachedBoxes) {}
#endif

};
}; // namespace kvcache
}; // adios2
#endif // ADIOS2_KVCACHEMETADATA_H
