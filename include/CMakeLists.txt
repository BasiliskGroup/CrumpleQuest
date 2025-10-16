# include glfw
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(GLFW_BUILD_WAYLAND ON CACHE BOOL "")
    set(GLFW_BUILD_X11    OFF CACHE BOOL "")
endif()

include(FetchContent)

# GLFW
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(glfw)

# GLM
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/icaven/glm.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(glm)

# ASSIMP
FetchContent_Declare(
  assimp
  GIT_REPOSITORY https://github.com/assimp/assimp.git
  GIT_TAG        v6.0.2
)
FetchContent_MakeAvailable(assimp)

# (Optional but recommended: xsimd for vectorization)
FetchContent_Declare(
  xsimd
  GIT_REPOSITORY https://github.com/xtensor-stack/xsimd.git
  GIT_TAG        11.1.0
)
FetchContent_MakeAvailable(xsimd)

# TBB
set(TBB_TEST OFF CACHE BOOL "" FORCE)
set(TBB_TESTING OFF CACHE BOOL "" FORCE) 
set(TBB_EXAMPLES OFF CACHE BOOL "" FORCE) 
set(TBB_STRICT OFF CACHE BOOL "" FORCE)
set(TBB_DISABLE_TESTS ON CACHE BOOL "" FORCE)

FetchContent_Declare(
    tbb
    GIT_REPOSITORY https://github.com/uxlfoundation/oneTBB
    GIT_TAG        master
)
FetchContent_MakeAvailable(tbb)