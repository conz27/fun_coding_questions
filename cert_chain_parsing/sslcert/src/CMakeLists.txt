cmake_minimum_required(VERSION 3.0.2)

project(certinfo)

add_compile_options(-O3)

set(REQUIRED_LIBS ssl crypto)

foreach(lib ${REQUIRED_LIBS})
    find_library(${lib} NAMES lib${lib}.a)
    if(${${lib}} MATCHES "-NOTFOUND")
        message(FATAL_ERROR "failed to find required library: lib${lib}.a")
    else()
        message(STATUS "lib${lib}.a = ${${lib}}")
    endif()
endforeach()

set(SOURCES certinfo.c)

include_directories(${PROJECT_SOURCE_DIR})
add_executable(certinfo ${SOURCES})
target_link_libraries(certinfo ${REQUIRED_LIBS})
