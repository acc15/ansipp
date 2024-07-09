cmake_minimum_required(VERSION 3.29.6)
project(ansipp VERSION 0.1.0 LANGUAGES CXX)
include(cmake/helpers.cmake)
configure_defaults(WITH_SHARED_LIBS)

add_library(ansipp)
target_sources(ansipp PRIVATE 
    ${SRC}/ansipp/ts_opt.hpp
    ${SRC}/ansipp/error.cpp
    ${SRC}/ansipp/io.cpp
    ${SRC}/ansipp/stream.cpp
    ${SRC}/ansipp/terminal.cpp
    ${SRC}/ansipp/cursor.cpp
    ${SRC}/ansipp/attrs.cpp
    ${SRC}/ansipp/restore.hpp
    ${SRC}/ansipp/restore.cpp
    ${SRC}/ansipp/init.cpp
)

target_sources(ansipp PUBLIC FILE_SET HEADERS BASE_DIRS ${INC} FILES
    ${INC}/ansipp/error.hpp
    ${INC}/ansipp/io.hpp
    ${INC}/ansipp/stream.hpp
    ${INC}/ansipp/config.hpp
    ${INC}/ansipp/terminal.hpp
    ${INC}/ansipp/cursor.hpp
    ${INC}/ansipp/attrs.hpp
    ${INC}/ansipp/restore.hpp
    ${INC}/ansipp/init.hpp
    ${INC}/ansipp.hpp
)

testing(TARGETS ansipp SOURCES 
    ${TEST}/ansipp/attrs.cpp
    ${TEST}/ansipp/cursor.cpp
)

option(BUILD_DEMOS "Build demo programs" ${PROJECT_IS_TOP_LEVEL})
if(BUILD_DEMOS)
    add_executable(ansipp_demo_simple demos/simple.cpp)
    target_link_libraries(ansipp_demo_simple ansipp)

    add_executable(ansipp_demo_snake demos/snake.cpp)
    target_link_libraries(ansipp_demo_snake ansipp)
    
    add_executable(ansipp_demo_readkey demos/readkey.cpp)
    target_link_libraries(ansipp_demo_readkey ansipp)

    add_executable(ansipp_demo_nonblock demos/nonblock.cpp)
    target_link_libraries(ansipp_demo_nonblock ansipp)
endif()