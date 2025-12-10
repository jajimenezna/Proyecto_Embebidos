cmake_minimum_required(VERSION 3.10)
project(drowsiness_detector C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2")

include_directories(${CMAKE_SOURCE_DIR}/include)

# Librería solo con algoritmo (sin hardware)
add_library(drowsiness SHARED
    src/drowsiness_c.c
    src/drowsiness_wrapper.c
)

target_link_libraries(drowsiness m)

install(TARGETS drowsiness DESTINATION lib)
