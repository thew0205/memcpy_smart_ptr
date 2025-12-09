cmake_minimum_required(VERSION 3.13)
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
set(TESTS OFF CACHE BOOL "Build tests" FORCE)

project(memcpy_smart_ptr C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

option(MEMCPY_SMART_PTR_TESTS "Build memcpy smart pointer test codes" OFF)

if(MEMCPY_SMART_PTR_TESTS)

    include(FetchContent)

    FetchContent_Declare(
        CppUTest
        GIT_REPOSITORY https://github.com/cpputest/cpputest.git
        GIT_TAG v4.0
    )

    FetchContent_MakeAvailable(CppUTest)
    # add_subdirectory(cpputest)

    target_compile_definitions(CppUTest PUBLIC CPPUTEST_USE_MEM_LEAK_DETECTION)
    add_executable(memcpy_smart_ptr_tests
        tests/test_unique.cpp
        tests/test_shared.cpp
        tests/test_main.cpp
    )




    target_compile_options(memcpy_smart_ptr_tests PUBLIC -fsanitize=address)
    target_link_options(memcpy_smart_ptr_tests PUBLIC -fsanitize=address)
    target_compile_definitions(memcpy_smart_ptr_tests PRIVATE

    )



    target_link_libraries(memcpy_smart_ptr_tests PRIVATE
        memcpy_smart_ptr
        CppUTest
        CppUTestExt
    )
endif(MEMCPY_SMART_PTR_TESTS)


