#ifndef ERASE_CHUNKS_H
#define ERASE_CHUNKS_H

#include "util/includes.h"
#include "shapes/mesh.h"

/**
 * @brief 
 * 
 * @tparam T 
 * @tparam MetaTypes 
 * @param start 
 * @param length 
 * @param toDelete 
 * @param meshes 
 * @param metas 
 */
template <typename T, typename... MetaTypes>
void eraseChunks(
    std::vector<T>& final,
    std::vector<uint>& start,
    std::vector<uint>& length,
    const std::vector<uint>& toDelete,
    std::unordered_map<uint32_t, Mesh*>& meshes,
    std::vector<MetaTypes>&... metas
) {
    if (toDelete.empty()) return;

    const uint origChunks = start.size();

    // Validate per-chunk vectors
    auto checkSize = [&](auto &v) {
        if (v.size() != origChunks)
            throw std::invalid_argument("All meta vectors must have the same length as start/length");
    };
    (checkSize(metas), ...);

    // Mark deleted chunks
    std::vector<char> isDeleted(origChunks, false);
    for (auto idx : toDelete)
        if (idx < origChunks) isDeleted[idx] = true;

    uint writePos = 0;
    uint newCount = 0;

    // We'll build a remapping table: old index → new index (or -1 if deleted)
    std::vector<int> newIndex(origChunks, -1);

    for (uint i = 0; i < origChunks; ++i) {
        if (isDeleted[i]) continue;

        const uint s   = start[i];
        const uint len = length[i];

        // Move chunk data
        if (s != writePos)
            std::move(final.begin() + s, final.begin() + s + len, final.begin() + writePos);

        // Move metadata
        ((metas[newCount] = std::move(metas[i])), ...);

        start[newCount]  = writePos;
        length[newCount] = len;
        newIndex[i] = static_cast<int>(newCount);

        writePos += len;
        ++newCount;
    }

    // Resize everything down
    final.resize(writePos);
    start.resize(newCount);
    length.resize(newCount);
    (metas.resize(newCount), ...);

    // --- Remap the unordered_map keys ---
    std::unordered_map<uint32_t, Mesh*> newMeshes;
    newMeshes.reserve(meshes.size());

    for (auto& [oldKey, ptr] : meshes) {
        int ni = (oldKey < newIndex.size()) ? newIndex[oldKey] : -1;
        if (ni >= 0)
            newMeshes[static_cast<uint32_t>(ni)] = ptr;
    }

    meshes.swap(newMeshes);
}

#endif