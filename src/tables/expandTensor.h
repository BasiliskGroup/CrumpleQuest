#ifndef EXPAND_TENSOR_H
#define EXPAND_TENSOR_H

#include "util/includes.h"

template <typename... T>
void expandTensors(const uint size, const uint newCapacity, std::vector<T>&... tensors) {
    ( tensors.resize(newCapacity), ... );
}

#endif