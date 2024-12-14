cmake_minimum_required(VERSION 3.22)
if(NOT DEFINED CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 23)
endif()

option(BUILD_SHARED_LIBS "Build shared library" OFF)

project(
  cutils
  VERSION 0.1.0
  LANGUAGES C)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()
set(WARNING_FLAGS
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -Wunused
    -Wuninitialized
    -Wshadow
    -Wconversion
    -Wduplicated-cond
    -Wduplicated-branches
    -Wlogical-op
    -Wsign-conversion
    -Wfloat-equal
    -Wformat=2
    -Wvla
    -Wcast-align
    -Wcast-qual
    -Wmissing-declarations
    -Wmissing-prototypes
    -Wstrict-prototypes
    -Wpointer-arith
    -Wredundant-decls
    -Wundef
    -Wwrite-strings
    -Wbad-function-cast
    -Wjump-misses-init
    -Wmissing-field-initializers
    -Wnested-externs
    -Wstrict-overflow=5
    -Wno-missing-braces
    -Wnull-dereference
    -Wdouble-promotion
    -Wformat-overflow=2)

file(GLOB_RECURSE CUTILS_LIB_SOURCES CONFIGURE_DEPENDS src/*.c)

add_library(cutils ${CUTILS_LIB_SOURCES})

if(BUILD_SHARED_LIBS)
  set_property(TARGET cutils PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

target_include_directories(cutils PUBLIC includes)
target_compile_options(cutils PRIVATE ${WARNING_FLAGS} $<$<CONFIG:Debug>:-g>
                                      $<$<CONFIG:Release>:-O2>)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  file(GLOB_RECURSE CUTILS_TEST_SOURCES CONFIGURE_DEPENDS test/*.c)
  add_executable(test ${CUTILS_TEST_SOURCES})
  target_link_libraries(test PRIVATE cutils)
  target_include_directories(test PRIVATE includes test)
  target_compile_options(test PRIVATE ${WARNING_FLAGS} -g)
endif()
