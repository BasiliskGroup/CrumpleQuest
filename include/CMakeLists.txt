# include glfw
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "")
    set(GLFW_BUILD_X11    ON CACHE BOOL "")
endif()

include(FetchContent)

# glm
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/icaven/glm.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(glm)

# Yay Yippee Horray! I love Basilisk Engine <3 :D
FetchContent_Declare(
  basilisk
  GIT_REPOSITORY https://github.com/BasiliskGroup/BasiliskEngine.git
  GIT_TAG        main
)
FetchContent_MakeAvailable(basilisk)