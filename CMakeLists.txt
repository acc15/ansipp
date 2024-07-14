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
    ${INC}/ansipp/vec.hpp
    ${INC}/ansipp/error.hpp
    ${INC}/ansipp/io.hpp
    ${INC}/ansipp/stream.hpp
    ${INC}/ansipp/config.hpp
    ${INC}/ansipp/terminal.hpp
    ${INC}/ansipp/cursor.hpp
    ${INC}/ansipp/attrs.hpp
    ${INC}/ansipp/restore.hpp
    ${INC}/ansipp/init.hpp
    ${INC}/ansipp/util.hpp
    ${INC}/ansipp.hpp
)

testing(TARGETS ansipp SOURCES 
    ${TEST}/ansipp/attrs.cpp
    ${TEST}/ansipp/cursor.cpp
    ${TEST}/ansipp/vec.cpp
)

option(BUILD_DEMOS "Build demo programs" ${PROJECT_IS_TOP_LEVEL})
if(BUILD_DEMOS)

    function(add_demo name)
        add_executable(ansipp_demo_${name} demos/${name}.cpp)
        target_link_libraries(ansipp_demo_${name} ansipp)
    endfunction()

    add_demo(simple)
    add_demo(snake)
    add_demo(readkey)
    add_demo(nonblock)
    add_demo(resize)
    add_demo(dead_pixels)
    if (WIN32)
        add_demo(windows_bug)
        add_demo(windows_terminal_performance)
    endif()

endif()