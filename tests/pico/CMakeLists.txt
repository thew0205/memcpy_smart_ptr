# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if(EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

set(TESTS OFF CACHE BOOL "Disable tests" FORCE)

# Pull in Raspberry Pi Pico SDK (must be before project)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

set(TESTS OFF CACHE BOOL "Disable tests" FORCE)

project(memcpy_smart_ptr_pico_tests C CXX ASM)


# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()


add_subdirectory(../../ ${CMAKE_CURRENT_BINARY_DIR}/memcpy_smart_ptr)
include(FetchContent)

FetchContent_Declare(
    CppUTest
    GIT_REPOSITORY https://github.com/cpputest/cpputest.git
    GIT_TAG v4.0
)

FetchContent_MakeAvailable(CppUTest)
# add_subdirectory(cpputest)

target_compile_definitions(CppUTest PUBLIC CPPUTEST_USE_MEM_LEAK_DETECTION)
add_executable(memcpy_smart_ptr_pico_tests
    ../test_unique.cpp
    ../test_shared.cpp
    test_main.cpp
)

target_compile_definitions(memcpy_smart_ptr_pico_tests PUBLIC PICO_CXX_DISABLE_ALLOCATION_OVERRIDES)



target_compile_options(memcpy_smart_ptr_pico_tests PUBLIC )
target_link_options(memcpy_smart_ptr_pico_tests PUBLIC)
target_compile_definitions(memcpy_smart_ptr_pico_tests PRIVATE

)



target_link_libraries(memcpy_smart_ptr_pico_tests PRIVATE
    memcpy_smart_ptr
    pico_stdlib
    CppUTest
    CppUTestExt
)

#output options
pico_enable_stdio_usb(memcpy_smart_ptr_pico_tests 1)
pico_enable_stdio_uart(memcpy_smart_ptr_pico_tests 1)

#ouputs .bin, .elf, .uf2 files
pico_add_extra_outputs(memcpy_smart_ptr_pico_tests)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--print-memory-usage")
