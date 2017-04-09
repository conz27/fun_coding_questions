cmake_minimum_required(VERSION 3.0.2)

project(gf2_matrix_ops)

# For Debugging:
#add_compile_options(-g)
#add_compile_options(-O0)

# For Performance:
add_compile_options(-O3)

set(SOURCES ${PROJECT_SOURCE_DIR}/src/gf2_matrix.c
            ${PROJECT_SOURCE_DIR}/src/main.c)

include_directories(${PROJECT_SOURCE_DIR}/include)
add_executable(gf2_matrix_ops ${SOURCES})
