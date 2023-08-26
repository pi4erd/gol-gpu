cmake_minimum_required(VERSION 3.20)

project(game-of-life VERSION 0.1.0)

include_directories(include)

find_package(OpenGL REQUIRED)
find_package(glfw3)
find_package(GLEW)

if(TARGET glfw)
    include_directories(${GLFW_INCLUDE_DIRS})
else()
    add_subdirectory(external/glfw)
    include_directories(external/glfw/include)
endif()

if(GLEW_FOUND)
    include_directories(${GLEW_INCLUDE_DIRS})
else()
    add_subdirectory(external/glew/build/cmake)
    include_directories(external/glew/include)
endif()

include_directories(external/imgui)
add_subdirectory(external/imgui)
add_subdirectory(src)
