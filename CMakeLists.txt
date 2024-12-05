cmake_minimum_required(VERSION 3.22)

if (NOT DEFINED CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 23)
endif()

project(project_name VERSION 0.0.0 LANGUAGES C)

set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
  -Wall -Wextra -Wpedantic -Werror
  -Wunused -Wuninitialized -Wshadow -Wconversion -Wduplicated-cond -Wduplicated-branches
  -Wlogical-op -Wsign-conversion -Wfloat-equal -Wformat=2 -Wvla
  -Wcast-align -Wcast-qual -Wmissing-declarations -Wmissing-prototypes -Wstrict-prototypes
  -Wpointer-arith -Wredundant-decls -Wundef -Wwrite-strings -Wbad-function-cast
  -Wjump-misses-init -Wmissing-field-initializers -Wnested-externs -Wstrict-overflow=5
  -Wno-missing-braces -Wnull-dereference -Wdouble-promotion -Wformat-overflow=2
)

set(CMAKE_BUILD_TYPE Debug)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  add_compile_options(-g)
endif()

if(CMAKE_BUILD_TYPE STREQUAL Release)
  add_compile_options(-Ofast)
endif()
