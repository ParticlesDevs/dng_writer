cmake_minimum_required(VERSION 3.4.1)
project(dngwriter)
set(CMAKE_CXX_STANDARD 23)
SET(MY_C_AND_CXX_FLAGS  -Werror, -Wdeprecated-register, -Wc++11-narrowing, -Wconstant-conversion -Wformat -Wformat-extra-args -Wunused-value -Wmacro-redefined)

SET(MY_C_FLAGS   ${MY_C_FLAGS}   ${MY_C_AND_CXX_FLAGS})
SET(MY_CXX_FLAGS ${MY_CXX_FLAGS} ${MY_C_AND_CXX_FLAGS})

include(loadDeps.cmake)
include_directories(dngwriter deps)

add_library(dngwriter SHARED
        dngwriter/DngWriter.cpp
        deps/lj92.c
)

include_directories(${TIFF_INCLUDE_DIR})

install(TARGETS dngwriter DESTINATION lib)

target_link_libraries(dngwriter tiff)

add_executable(dngWritertest
        test.cpp
)

target_link_libraries(dngWritertest tiff dngwriter)