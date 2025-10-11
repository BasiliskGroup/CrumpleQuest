#ifndef COMPACT_TENSOR_H
#define COMPACT_TENSOR_H

#include "util/includes.h"

template <typename BoolTensor, typename... Tensors>
void compactTensors(const BoolTensor& toDelete, uint size, Tensors&... tensors)
{
    uint dst = 0;

    // Helper lambda for one tensor
    auto moveRow = [&](auto& tensor, uint dst, uint src) {
        // Compute number of elements in one row (product of remaining dimensions)
        size_t row_size = std::accumulate(
            tensor.shape().begin() + 1, tensor.shape().end(), (uint) 1, std::multiplies<size_t>());
        std::memmove(
            tensor.data() + dst * row_size,
            tensor.data() + src * row_size,
            row_size * sizeof(typename std::decay_t<decltype(tensor)>::value_type)
        );
    };

    for (uint src = 0; src < size; ++src) {
        if (!toDelete(src)) {
            if (dst != src) {
                (moveRow(tensors, dst, src), ...);
            }
            ++dst;
        }
    }
}

uint numValid(xt::xtensor<bool, 1> toDelete, uint size);

#endif