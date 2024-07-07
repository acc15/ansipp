cmake_minimum_required(VERSION 3.29.6)
project(ansipp VERSION 0.1.0 LANGUAGES CXX)
include(cmake/helpers.cmake)
configure_defaults(WITH_SHARED_LIBS)

add_library(ansipp)
target_sources(ansipp PRIVATE 
    ${SRC}/ansipp/ts_opt.hpp
    ${SRC}/ansipp/error.cpp
    ${SRC}/ansipp/io.cpp
    ${SRC}/ansipp/attrs.cpp
    ${SRC}/ansipp.cpp
)

target_sources(ansipp PUBLIC FILE_SET HEADERS BASE_DIRS ${INC} FILES
    ${INC}/ansipp/error.hpp
    ${INC}/ansipp/io.hpp
    ${INC}/ansipp/config.hpp
    ${INC}/ansipp/attrs.hpp
    ${INC}/ansipp.hpp
)

testing(TARGETS ansipp SOURCES 
    ${TEST}/ansipp.cpp
)

option(BUILD_DEMO "Build demo program" ${PROJECT_IS_TOP_LEVEL})
if(BUILD_DEMO)
    add_executable(ansipp_demo demo/demo.cpp)
    target_link_libraries(ansipp_demo ansipp)
endif()