cmake_minimum_required(VERSION 3.12)

project(trap_test LANGUAGES CXX)

add_subdirectory(external)

add_executable(trap_test
    main.cpp
)

target_link_libraries(trap_test PRIVATE
    Catch2::Catch2
)
