cmake_required_minimium(VERSION 3.0.2)

project(fun_coding_questions)

add_compiler_options(-g)
add_compiler_options(-Wall)
add_compiler_options(-Wextra)
add_compiler_options(-fstack-protector)

add_subdirectory(multiply_large_integers)
#add_subdirectory(cert_chain_parsing)
#add_subdirectory(gf2_matrix_ops)
