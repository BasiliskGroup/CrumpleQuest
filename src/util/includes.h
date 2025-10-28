#ifndef INCLUDES_H
#define INCLUDES_H

#include "constants.h"

// glm
#include <glm/glm.hpp>

// look for future changes to glm experimental
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/quaternion.hpp> // these are different 
#include <glm/gtc/quaternion.hpp> // ^
#include <glm/gtc/matrix_transform.hpp>


// data structures
#include <vector>
#include <queue>
#include <set>

#include <algorithm>

// earcut
#include <earcut/earcut.hpp>

// shorthand names
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat2x2 = glm::mat2x2;
using mat3x3 = glm::mat3x3;
using mat4x4 = glm::mat4x4;
using quat = glm::quat;

using uint = unsigned int;
using ushort = unsigned short;

// AoS Types
using Vec2Triplet = std::array<vec2, 3>;
using Vec2Pair = std::array<vec2, 2>;
using FloatPair = std::array<float, 2>;

#endif