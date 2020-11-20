cmake_minimum_required(VERSION 3.17)
project(pixies)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_BUILD_TYPE Debug)

include_directories(../kki_util/include)
include_directories(vendor/glad/include)
include_directories(vendor/FastNoise)

add_executable(pixies
        src/main.cpp
        vendor/glad/glad.c

        src/util/graphics_error.h

        src/Context.h
        src/Context.cpp
        src/Shader.h
        src/Shader.cpp
        src/util/gl_types.h src/util/gl_types.cpp)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(vendor/glfw-3.3.2)
target_link_libraries(pixies glfw)

add_subdirectory(vendor/glm)
target_link_libraries(pixies glm)

find_package(OpenGL REQUIRED)
target_link_libraries(pixies OpenGL::GL)