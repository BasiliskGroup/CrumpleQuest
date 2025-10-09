#ifndef EXPAND_TENSOR_H
#define EXPAND_TENSOR_H

#include "util/includes.h"
#include "util/indexed.h"

/**
 * @brief moves a smaller tensor into a larger one of a give size. This should be used for memory doubling. 
 * 
 * @tparam Tensor 
 * @param tensor 
 * @param size 
 * @param newCapacity 
 */
template <typename Tensor>
void expandTensor(Tensor& tensor, const uint size, const uint newCapacity)
{
    // shape & rank
    auto oldShape = tensor.shape();
    uint rank = tensor.dimension();

    // guard: size must not exceed old first-dimension
    if (size > oldShape[0]) throw std::runtime_error("size > old first-dimension");

    // build new shape
    auto newShape = oldShape;
    newShape[0] = newCapacity;

    // create the new tensor (default-initialized)
    Tensor newTensor = Tensor::from_shape(newShape);

    // compute number of elements per "slice" along axis 0 (product of remaining dims)
    uint inner_count = 1;
    for (uint i = 1; i < rank; ++i) inner_count *= oldShape[i];

    // copy contiguous block: first `size` slices
    // NOTE: this assumes row-major contiguous storage (xtensor default)
    auto* src = tensor.data();
    auto* dst = newTensor.data();
    std::copy_n(src, size * inner_count, dst);

    // move new tensor back
    tensor = std::move(newTensor);
}

template <typename... Tensors>
void expandTensors(const uint size, const uint newCapacity, Tensors&... tensors) {
    ( expandTensor(tensors, size, newCapacity), ... );
}

#endif