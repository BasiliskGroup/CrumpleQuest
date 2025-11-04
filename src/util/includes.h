// util/includes.h
#ifndef INCLUDES_H
#define INCLUDES_H

#include <basilisk/basilisk.h>
using namespace bsk;

#include <earcut/earcut.hpp>

#include "util/constants.h"
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <vector>
#include <queue>
#include <set>
#include <unordered_map>
#include <algorithm>

using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat2x2 = glm::mat2x2;
using mat3x3 = glm::mat3x3;
using mat4x4 = glm::mat4x4;
using quat = glm::quat;

using uint = unsigned int;
using ushort = unsigned short;

using Vec2Triplet = std::array<vec2, 3>;
using Vec2Pair = std::array<vec2, 2>;
using FloatPair = std::array<float, 2>;

// Proper hash functor for glm::vec2

#endif
