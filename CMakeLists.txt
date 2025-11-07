cmake_minimum_required(VERSION 3.14)
project(CrumpleQuest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ======================================================
# Dependencies
# ======================================================
include(FetchContent)

# Basilisk Engine (includes Assimp)
FetchContent_Declare(
    basilisk
    GIT_REPOSITORY https://github.com/BasiliskGroup/BasiliskEngine.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(basilisk)

# Add this AFTER FetchContent_MakeAvailable(basilisk)
message(STATUS "=== DEBUG INFO ===")
message(STATUS "basilisk_SOURCE_DIR: ${basilisk_SOURCE_DIR}")
message(STATUS "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")

# Check if the path exists
set(EARCUT_PATH "${basilisk_SOURCE_DIR}/_deps/assimp-src/contrib/earcut-hpp")
if(EXISTS ${EARCUT_PATH})
    message(STATUS "EARCUT PATH EXISTS: ${EARCUT_PATH}")
else()
    message(STATUS "EARCUT PATH DOES NOT EXIST: ${EARCUT_PATH}")
endif()

# Try to find it
file(GLOB_RECURSE EARCUT_FILES "${CMAKE_BINARY_DIR}/*earcut.hpp")
message(STATUS "Found earcut files: ${EARCUT_FILES}")

# ======================================================
# Source files and executable
# ======================================================
file(GLOB_RECURSE SRC_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(game ${SRC_FILES})

# Add include directories
# Add include directories pointing to Assimp sources
target_include_directories(game PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/_deps/assimp-src/contrib/earcut-hpp
)

# Link Basilisk
target_link_libraries(game PRIVATE basilisk)

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
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders 
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/textures 
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/models 
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
