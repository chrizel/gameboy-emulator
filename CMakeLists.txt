cmake_minimum_required(VERSION 2.6)
project(gb)

set(SOURCE
    cpu.cc
    debugger.cc
    gameboy.cc
    instructions.cc
    instructionset.cc
    main.cc
    memory.cc
    word.cc
)

set(HEADERS
    cpu.h
    debugger.h
    gameboy.h
    instructions.h
    instructionset.h
    memory.h
    references.h
    word.h
)

source_group("Headers" FILES ${HEADERS})

find_package(Boost 1.47.0 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

link_libraries(${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} ${OPENGL_LIBRARY})
include_directories(${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR} ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_command(
    OUTPUT base_instructionset.h
    COMMAND instructionset_generator ${CMAKE_CURRENT_SOURCE_DIR}/base_instructionset.txt ${CMAKE_CURRENT_BINARY_DIR}/base_instructionset.h
    DEPENDS instructionset_generator ${CMAKE_CURRENT_SOURCE_DIR}/base_instructionset.txt
)

add_custom_command(
    OUTPUT cb_instructionset.h
    COMMAND instructionset_generator ${CMAKE_CURRENT_SOURCE_DIR}/cb_instructionset.txt ${CMAKE_CURRENT_BINARY_DIR}/cb_instructionset.h
    DEPENDS instructionset_generator ${CMAKE_CURRENT_SOURCE_DIR}/cb_instructionset.txt
)

add_executable(gb ${SOURCE} ${HEADERS} base_instructionset.h cb_instructionset.h)
add_executable(instructionset_generator instructionset_generator.cc)
