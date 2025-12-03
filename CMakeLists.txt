cmake_minimum_required(VERSION 3.20)
project(CrumpleQuest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# ======================================================
# Basilisk Engine (includes Assimp + Clipper2 internally)
# ======================================================
FetchContent_Declare(
    basilisk
    GIT_REPOSITORY https://github.com/BasiliskGroup/BasiliskEngine.git
    GIT_TAG main
)
FetchContent_MakeAvailable(basilisk)

# NOTE:
# Basilisk exposes its own clipper2 target via FetchContent.
# So we do NOT declare or build our own copy.
# ======================================================

# ======================================================
# Source files and executable
# ======================================================
file(GLOB_RECURSE SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# External include folder sources (kept separate)
file(GLOB_RECURSE INCLUDE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.cpp
)

add_executable(game ${SRC_FILES} ${INCLUDE_FILES})

target_include_directories(game PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/_deps/assimp-src/contrib/earcut-hpp
)

# ======================================================
# Link Basilisk (automatically brings clipper2)
# ======================================================
target_link_libraries(game
    PRIVATE
        basilisk        # includes clipper2, glm, assimp, stb, glad, glfw, etc.
)

# ======================================================
# AddressSanitizer (Debug)
# ======================================================
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU" AND NOT CMAKE_CROSSCOMPILING)
    target_compile_options(game PRIVATE
        -fsanitize=address
        -fno-omit-frame-pointer
        -g
    )
    target_link_options(game PRIVATE
        -fsanitize=address
        -fno-omit-frame-pointer
    )
    set(ENV{ASAN_OPTIONS} "detect_leaks=1:halt_on_error=0:verbosity=1")
endif()

# ======================================================
# Resources
# ======================================================
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders  DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/textures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/models   DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/sounds   DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/art      DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
