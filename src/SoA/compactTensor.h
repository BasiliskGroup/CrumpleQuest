#ifndef COMPACT_TENSOR_H
#define COMPACT_TENSOR_H

#include "util/includes.h"

template <typename BoolTensor, typename... Tensors>
void compactTensors(
    const BoolTensor& toDelete,
    uint size,
    Tensors&... tensors)
{
    uint dst = 0;

    for (uint src = 0; src < size; ++src) {
        if (!toDelete(src)) {
            if (dst != src) {
                // Move all tensors' row src → dst
                ((xt::view(tensors, dst) = xt::view(tensors, src)), ...);
            }
            ++dst;
        }
    }
}

uint numValid(xt::xtensor<bool, 1> toDelete, uint size);

#endif