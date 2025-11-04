cmake_minimum_required(VERSION 3.14)
project(CrumpleQuest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ======================================================
# Dependencies
# ======================================================

include(FetchContent)

# Fetch Basilisk Engine
FetchContent_Declare(
    basilisk
    GIT_REPOSITORY https://github.com/BasiliskGroup/BasiliskEngine.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(basilisk)

# ======================================================
# Header-only Libraries
# ======================================================

# Create an interface target for earcut.hpp
add_library(earcut_hpp INTERFACE)
target_include_directories(earcut_hpp INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ======================================================
# Source files and executable
# ======================================================

file(GLOB_RECURSE SRC_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(game ${SRC_FILES})

# Add src folder to include directories
target_include_directories(game PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link Basilisk and earcut_hpp
target_link_libraries(game PUBLIC 
    basilisk
    earcut_hpp
)

# ======================================================
# AddressSanitizer (Debug)
# ======================================================

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU" AND NOT CMAKE_CROSSCOMPILING)
    target_compile_options(game PRIVATE -fsanitize=address -fno-omit-frame-pointer -g)
    target_link_options(game PRIVATE -fsanitize=address -fno-omit-frame-pointer)
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
