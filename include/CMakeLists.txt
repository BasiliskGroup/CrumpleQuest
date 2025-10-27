# include glfw
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "")
    set(GLFW_BUILD_X11    ON CACHE BOOL "")
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
  GIT_TAG        13.0.0
)
FetchContent_MakeAvailable(xsimd)