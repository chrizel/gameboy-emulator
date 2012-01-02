cmake_minimum_required(VERSION 2.6)
project(gb)

find_package(GLUT)
find_package(OpenGL)

add_executable(gb cpu.cc debugger.cc gameboy.cc main.cc memory.cc)
target_link_libraries(gb ${GLUT_LIBRARY} ${OPENGL_LIBRARY})
