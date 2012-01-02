cmake_minimum_required(VERSION 2.6)
project(gb)

find_package(GLUT)
find_package(OpenGL)

#find_package(Boost 1.48.0)

link_libraries(${GLUT_LIBRARY} ${OPENGL_LIBRARY})
#include_directories(${Boost_INCLUDE_DIRS})

add_executable(gb cpu.cc debugger.cc gameboy.cc main.cc memory.cc word.cc)
