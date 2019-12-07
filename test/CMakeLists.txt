cmake_minimum_required(VERSION 3.12)

project(trap_test LANGUAGES CXX)

add_subdirectory(external)

add_executable(trap_test
    main.cpp
    test_catch2.cpp
    test_trap.cpp
)

target_include_directories(trap_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(trap_test PRIVATE
    Catch2::Catch2
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
