cmake_minimum_required(VERSION 3.30.0)
project(ansipp VERSION 0.1.0 LANGUAGES CXX)
include(cmake/helpers.cmake)
configure_defaults(WITH_SHARED_LIBS)

add_library(ansipp)
target_sources(ansipp PRIVATE 
    ${SRC}/ansipp/ts_opt.hpp
    ${SRC}/ansipp/error.cpp
    ${SRC}/ansipp/io.cpp
    ${SRC}/ansipp/terminal.cpp
    ${SRC}/ansipp/cursor.cpp
    ${SRC}/ansipp/attrs.cpp
    ${SRC}/ansipp/restore.hpp
    ${SRC}/ansipp/restore.cpp
    ${SRC}/ansipp/init.cpp
)

target_sources(ansipp PUBLIC FILE_SET HEADERS BASE_DIRS ${INC} FILES
    ${INC}/ansipp/integral.hpp
    ${INC}/ansipp/charbuf.hpp
    ${INC}/ansipp/esc.hpp 
    ${INC}/ansipp/vec.hpp
    ${INC}/ansipp/error.hpp
    ${INC}/ansipp/io.hpp
    ${INC}/ansipp/config.hpp
    ${INC}/ansipp/terminal.hpp
    ${INC}/ansipp/cursor.hpp
    ${INC}/ansipp/attrs.hpp
    ${INC}/ansipp/restore.hpp
    ${INC}/ansipp/init.hpp
    ${INC}/ansipp/util.hpp
    ${INC}/ansipp/mouse.hpp
    ${INC}/ansipp.hpp
)

testing(TARGETS ansipp SOURCES 
    ${TEST}/ansipp/attrs.cpp
    ${TEST}/ansipp/cursor.cpp
    ${TEST}/ansipp/vec.cpp
    ${TEST}/ansipp/format.cpp
    ${TEST}/ansipp/charbuf.cpp
    ${TEST}/ansipp/integral.cpp
)

configure_install(ansipp)

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
    add_demo(dead_pixels)
endif()

option(BUILD_NCURSES_DEMOS "Build NCURSES demo programs (for comparison)" OFF)
if (BUILD_NCURSES_DEMOS)
    set(CURSES_NEED_NCURSES TRUE)
    find_package(Curses REQUIRED)
    add_executable(ansipp_demo_snake_ncurses demos/snake_ncurses.cpp)
    target_include_directories(ansipp_demo_snake_ncurses PRIVATE ${CURSES_INCLUDE_DIRS})
    target_link_libraries(ansipp_demo_snake_ncurses ${CURSES_LIBRARIES})
endif()