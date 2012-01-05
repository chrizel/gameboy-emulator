cmake_minimum_required(VERSION 2.6)
project(gb)

set(SOURCE
    cpu.cc
    debugger.cc
    gameboy.cc
    main.cc
    memory.cc
    word.cc
)

set(HEADERS
    cpu.h
    debugger.h
    gameboy.h
    instructions.h
    memory.h
    references.h
    word.h
)

source_group("Headers" FILES ${HEADERS})

find_package(GLUT)
find_package(OpenGL)
#find_package(Boost 1.48.0)

link_libraries(${GLUT_LIBRARY} ${OPENGL_LIBRARY})
#include_directories(${Boost_INCLUDE_DIRS})

add_executable(gb ${SOURCE} ${HEADERS})
