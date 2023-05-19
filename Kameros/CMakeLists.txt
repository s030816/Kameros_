# CMakeList.txt : CMake project for Kameros, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.0)

project ("Kameros")


add_library(glad glad/glad.h glad/glad.c)
# Add source to this project's executable.
add_executable (Kameros main.cpp glad/glad.c Shader.cpp Renderer.cpp Camera.cpp)

find_package( OpenGL REQUIRED )
include_directories( ${OPENGL_INCLUDE_DIRS} )

target_link_libraries(Kameros ${CMAKE_CURRENT_SOURCE_DIR}/GLFW/glfw3.lib)
