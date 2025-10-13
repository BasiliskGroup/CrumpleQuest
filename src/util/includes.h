#ifndef INCLUDES_H
#define INCLUDES_H

// io
#include <iostream>
#include <string>

// errors
#include <stdexcept>

// standard data structures
#include <array>
#include <set>
#include <tuple>
#include <unordered_map>
#include <vector>

// helpful stuff
#include <memory> // I forgor what this does but it was in the 3d version somewhere
#include <optional> // check null returns (avoid using this)
#include <utility> // for std::move and std::pair
#include <cmath>
#include <algorithm>
#include <functional>
#include <numeric>
#include <cstdint>
#include <type_traits>

// import glad and glfw
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// import glm
#include <glm/glm.hpp>

// import xtensor
#include <xtensor/xtensor.hpp>
#include <xtensor/xarray.hpp>
#include <xtensor/xadapt.hpp>
#include <xtensor/xio.hpp>
#include <xtensor/xmasked_view.hpp>
#include <xtensor/xnoalias.hpp>
#include <xtensor/xstrided_view.hpp>
#include <xtensor/xview.hpp>

// look for future changes to glm experimental
#define GLM_ENABLE_EXPERIMENTAL 
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

// tbb


// import assimp Jonah Stuff
#include <assimp/scene.h>
#include <stb/stb_image.h>

#include "util/constants.h"

#define DEBUG_PRINT true
#define DEBUG_LINEAR_PRINT false

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

using Vec2Triplet = std::array<vec2, 3>;
using Vec2Pair = std::array<vec2, 2>;
using Vec3ROWS = std::array<vec2, ROWS>;
using Mat3x3ROWS = std::array<mat3x3, ROWS>;
using FloatROWS = std::array<float, ROWS>;

enum JType {
    JN,
    JT,
};

enum ForceType {
    NULL_FORCE,
    MANIFOLD,
    JOINT,
    SPRING,
    IGNORE_COLLISION
};

#endif