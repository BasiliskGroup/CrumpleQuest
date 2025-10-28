cmake_minimum_required(VERSION 3.14)
project(CrumpleQuest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies

# basilisk
include(FetchContent)
add_subdirectory(include)

# Executable
file(GLOB_RECURSE SRC_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp
)

add_executable(game ${SRC_FILES})
target_include_directories(game PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link deps
target_link_libraries(game
    glm
    basilisk
)

# -----------------------
# compile options
# -----------------------

target_compile_options(game PRIVATE
    # Optimization for Release builds only
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:Release>:-march=native>
    # -ffast-math
    # Windows equivalents (Release only to avoid conflict with /RTC1)
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/O2 /arch:AVX2>
    # Enable C++ alternative operators on Windows (all configs)
    $<$<CXX_COMPILER_ID:MSVC>:/permissive->
)

# -----------------------
# resources
# -----------------------

# Copy resources to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders/ 
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/shaders)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/textures/ 
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/textures)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/models/ 
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/models)

# For MSVC multi-config generators, also copy to Debug/Release subdirectories
if(MSVC)
    foreach(CONFIG Debug Release RelWithDebInfo MinSizeRel)
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders/ 
             DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG}/shaders)
        
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/textures/ 
             DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG}/textures)
        
        file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/models/ 
             DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG}/models)
    endforeach()
endif()

# TODO add same things for textures


# -----------------------
# debug
# -----------------------

# Enable AddressSanitizer in debug builds (GCC/Clang)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(game PUBLIC -fsanitize=address -fno-omit-frame-pointer -g)
    target_link_options(game PUBLIC -fsanitize=address -fno-omit-frame-pointer)
endif()

# MSVC debug configuration (uses /RTC1 by default, which conflicts with optimization)
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(game PRIVATE
        $<$<CONFIG:Debug>:/Zi>       # Debug information
        $<$<CONFIG:Debug>:/Od>       # Disable optimization (required for /RTC1)
    )
endif()