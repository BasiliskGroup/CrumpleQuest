#ifndef COMPACT_TENSOR_H
#define COMPACT_TENSOR_H

#include "util/includes.h"

template <typename... T>
void compactTensors(const std::vector<bool>& toDelete, uint size, std::vector<T>&... tensors)
{
    uint dst = 0;

    for (uint src = 0; src < size; ++src) {
        if (!toDelete[src]) {
            if (dst != src) {
                // Assign all tensors at once
                ((tensors[dst] = tensors[src]), ...);
            }
            ++dst;
        }
    }
}

uint numValid(const std::vector<bool>& toDelete, const uint size);

#endif