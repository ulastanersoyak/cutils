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

# the allocator and time code are always built, every other module can be
# left out to save flash, e.g. -DCUTILS_WITH_MAP=OFF
set(CUTILS_LIB_SOURCES src/allocator.c src/time.c)
set(CUTILS_MODULES
    arena
    expected
    list
    map
    pool
    pqueue
    queue
    stack
    string
    vector)
foreach(module IN LISTS CUTILS_MODULES)
  string(TOUPPER ${module} module_upper)
  option(CUTILS_WITH_${module_upper} "Build the ${module} module" ON)
  if(CUTILS_WITH_${module_upper})
    list(APPEND CUTILS_LIB_SOURCES src/${module}.c)
  endif()
endforeach()

add_library(cutils ${CUTILS_LIB_SOURCES})
add_library(cutils::cutils ALIAS cutils)

if(BUILD_SHARED_LIBS)
  set_property(TARGET cutils PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

target_include_directories(
  cutils PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/includes>
                $<INSTALL_INTERFACE:include>)
target_compile_options(cutils PRIVATE ${WARNING_FLAGS} $<$<CONFIG:Debug>:-g>
                                      $<$<CONFIG:Release>:-O2>)

include(GNUInstallDirs)
install(TARGETS cutils EXPORT cutils-targets)
install(DIRECTORY includes/cutils DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(
  EXPORT cutils-targets
  NAMESPACE cutils::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cutils
  FILE cutils-config.cmake)
