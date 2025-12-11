cmake_minimum_required(VERSION 3.10)
project(drowsiness_detector C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -fPIC")

add_library(drowsiness SHARED
    src/drowsiness_hybrid_final.c
)

target_link_libraries(drowsiness m)
