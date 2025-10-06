cmake_minimum_required(VERSION 3.14)
project(Basilisk)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies

# glad
add_library(glad STATIC include/glad/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# stb (header-only)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# glm, glfw, assimp, xsimd
include(FetchContent)
add_subdirectory(include)

# Executable
file(GLOB_RECURSE SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(engine ${SRC_FILES})
target_include_directories(engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link deps
target_link_libraries(engine
    glad
    glfw
    glm
    assimp
    stb
    xtl
    xtensor
    xsimd
    TBB::tbb
)

# -----------------------
# compile options
# -----------------------

# target_compile_options(engine PRIVATE
#     -O3
#     -ffast-math
#     -march=native
# )

# -----------------------
# resources
# -----------------------

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/shaders)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# TODO add same things for textures


# -----------------------
# debug
# -----------------------

# Enable AddressSanitizer in debug builds
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(engine PUBLIC -fsanitize=address -fno-omit-frame-pointer -g)
    target_link_options(engine PUBLIC -fsanitize=address -fno-omit-frame-pointer)
endif()