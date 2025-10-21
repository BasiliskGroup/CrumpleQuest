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
#include <queue>


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
#include <limits>
#include <fstream>
#include <sstream>

// import glad and glfw
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// import glm
#include <glm/glm.hpp>

// look for future changes to glm experimental
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

// tbb


// STB
#include <stb/stb_image.h>
#include <stb/stb_image_resize2.h>

// Assimp
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "constants.h"

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

// AoS Types
using Vec2Triplet = std::array<vec2, 3>;
using Vec2Pair = std::array<vec2, 2>;
using FloatPair = std::array<float, 2>;
using Vec3ROWS = std::array<vec3, ROWS>;
using Mat3x3ROWS = std::array<mat3x3, ROWS>;
using FloatROWS = std::array<float, ROWS>;

// Mini structs
struct CollisionIndexPair {
    uint bodyA;
    uint bodyB;
    uint manifold = -1;

    CollisionIndexPair(uint bodyA, uint bodyB) : bodyA(bodyA), bodyB(bodyB) {}
    CollisionIndexPair(uint bodyA, uint bodyB, uint manifold) : bodyA(bodyA), bodyB(bodyB), manifold(manifold) {}
};

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

struct Vertex {
    vec3 position;
    vec2 uv;
    vec3 normal;
};

#endif