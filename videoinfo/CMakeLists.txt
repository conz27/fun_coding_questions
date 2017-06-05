cmake_minimum_required(VERSION 3.0)

project(videoinfo)

add_compile_options(-std=c++11)

find_library(boost_program_opts NAMES libboost_program_options.a)
if(boost_program_opts MATCHES "-NOTFOUND")
    message(FATAL_ERROR "install libboost_program_options")
endif()

find_library(boost_filesystem NAMES libboost_filesystem.a)
if(boost_filesystem MATCHES "-NOTFOUND")
    message(FATAL_ERROR "install libboost_filesystem")
endif()

find_library(boost_system NAMES libboost_system.a)
if(boost_filesystem MATCHES "-NOTFOUND")
    message(FATAL_ERROR "install libboost_system")
endif()

include_directories(${PROJECT_SOURCE_DIR}/src)

set(SOURCES ${PROJECT_SOURCE_DIR}/src/main.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} ${boost_program_opts} ${boost_filesystem} ${boost_system})


install(TARGETS ${PROJECT_NAME}
        DESTINATION bin
	)

install(FILES ${PROJECT_SOURCE_DIR}/mpg_to_yuv.sh
	      ${PROJECT_SOURCE_DIR}/README.md
	DESTINATION bin
	)

set(CPACK_PACKAGE_NAME videoinfo)
set(CPACK_PACKAGE_VERSION 0.1.0)

include(CPack)
