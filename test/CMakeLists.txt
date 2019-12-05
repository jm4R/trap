cmake_minimum_required(VERSION 3.12)

project(trap_test LANGUAGES CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(trap_test
    main.cpp
)

target_link_libraries(trap_test
    ${CONAN_LIBS}
)
