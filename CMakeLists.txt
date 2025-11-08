cmake_minimum_required(VERSION 3.20)
project(CrumpleQuest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# ======================================================
# Basilisk Engine (includes Assimp)
# ======================================================
FetchContent_Declare(
    basilisk
    GIT_REPOSITORY https://github.com/BasiliskGroup/BasiliskEngine.git
    GIT_TAG main
)
FetchContent_MakeAvailable(basilisk)

# ======================================================
# Clipper2
# ======================================================
FetchContent_Declare(
    clipper2
    GIT_REPOSITORY https://github.com/AngusJohnson/Clipper2.git
    GIT_TAG main   # <- use 'main', not 'master'
)
FetchContent_MakeAvailable(clipper2)

# Build Clipper2 as a static library
file(GLOB CLIPPER2_SOURCES
    ${clipper2_SOURCE_DIR}/CPP/Clipper2Lib/src/*.cpp
)

add_library(clipper2 STATIC ${CLIPPER2_SOURCES})
target_include_directories(clipper2 PUBLIC
    ${clipper2_SOURCE_DIR}/CPP/Clipper2Lib/include
)
target_compile_features(clipper2 PUBLIC cxx_std_20)

# ======================================================
# Source files and executable
# ======================================================
file(GLOB_RECURSE SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(game ${SRC_FILES})

target_include_directories(game PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/_deps/assimp-src/contrib/earcut-hpp
)

# Link Basilisk + Clipper2
target_link_libraries(game PRIVATE basilisk clipper2)

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
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/textures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/models DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
