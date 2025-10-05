#ifndef HELPER_H
#define HELPER_H

#include "util/includes.h"
#include "util/indexed.h"

/**
 * @brief Copy row/slice `src` into `dst` for any tensor with rank >= 1
 * 
 * @param t 
 * @param dst 
 * @param src 
 * @return * template <typename Tensor> 
 */
template <typename Tensor>
void moveRow(Tensor& t, uint dst, uint src) {
    xt::view(t, dst) = xt::view(t, src);
}


/**
 * @brief Compacts a tensor into a single contiguous block of memory
 * 
 * @tparam Map uint -> Any
 * @tparam Tensors list of any shape of xtensor 
 * @param freeIndices 
 * @param capacity 
 * @param map 
 * @param tensors 
 */
template <typename Map, typename... Tensors>
void compactTensors(const std::set<uint>& freeIndices, uint capacity, Map& map, Tensors&... tensors)
{
    if (freeIndices.empty()) return;
    uint nextPos = 0;

    for (uint i = 0; i < capacity; ++i) {
        if (freeIndices.find(i) == freeIndices.end()) {
            if (nextPos != i) {
                ( moveRow(tensors, nextPos, i), ... );

                auto it = map.find(i);
                if (it != map.end()) {
                    auto value = it->second;
                    map.erase(it);
                    map[nextPos] = value;
                }
            }
            ++nextPos;
        }
    }
}

/**
 * @brief moves a smaller tensor into a larger one of a give size. This should be used for memory doubling. 
 * 
 * @tparam Tensor 
 * @param tensor 
 * @param size 
 * @param newCapacity 
 */
template <typename Tensor>
void expandTensor(Tensor& tensor, uint size, uint newCapacity)
{
    // Copy old shape and adjust first dimension
    auto newShape = tensor.shape();
    newShape[0] = newCapacity;

    // Create expanded tensor
    Tensor newTensor = Tensor::from_shape(newShape);

    // Copy old values into the expanded tensor
    xt::view(newTensor, xt::range(0, size), xt::all()) = tensor;

    // Move back
    tensor = std::move(newTensor);
}

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
    std::unordered_map<uint32_t, Indexed*>& meshes,
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
    std::unordered_map<uint32_t, Indexed*> newMeshes;
    newMeshes.reserve(meshes.size());

    for (auto& [oldKey, ptr] : meshes) {
        int ni = (oldKey < newIndex.size()) ? newIndex[oldKey] : -1;
        if (ni >= 0)
            newMeshes[static_cast<uint32_t>(ni)] = ptr;
    }

    meshes.swap(newMeshes);
}

#endif