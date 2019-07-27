cmake_minimum_required(VERSION 3.3)
project(crucian CXX)

set(CMAKE_VERBOSE_MAKEFILE OFF)

option(CRU_SHARED "Build shared or static library" ON)
option(CRU_PIC "Enable position independent code" OFF)
option(CRU_TEST "Build tests" ON)

string(TOUPPER ${CMAKE_SYSTEM_NAME} PLATFORM_UPPERCASE)
set(BITNESS 32)
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(BITNESS 64)
endif ()

set(src_compiler_definitions
        -DNTA_OS_${PLATFORM_UPPERCASE}
        -DNTA_ARCH_${BITNESS}
        -DNTA_INTERNAL
        -DNTA_ASM)

if (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    set(src_compiler_definitions ${src_compiler_definitions} -DNTA_ASSERTIONS_ON)
endif ()

add_definitions(${src_compiler_definitions})
file(GLOB_RECURSE cru_src "include/*.hpp" "src/*.cpp")
file(GLOB test_src "test/*.cpp")

if (CRU_SHARED)
    add_library(${PROJECT_NAME} SHARED ${cru_src})
else ()
    add_library(${PROJECT_NAME} STATIC ${cru_src})
endif ()

set(CMAKE_POSITION_INDEPENDENT_CODE CRU_PIC)
target_include_directories(${PROJECT_NAME} PUBLIC "include")

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 14
        CXX_EXTENSIONS OFF)

if (CRU_TEST)
    find_package(GTest REQUIRED)
    add_executable(tests ${test_src})
    target_link_libraries(tests GTest::GTest GTest::Main ${PROJECT_NAME})
    target_include_directories(tests PUBLIC "include")
endif ()
