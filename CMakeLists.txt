cmake_minimum_required(VERSION 3.10)
project(drowsiness_detector C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wall")

add_library(drowsiness SHARED src/drowsiness_c.c)
target_link_libraries(drowsiness m)

install(TARGETS drowsiness LIBRARY DESTINATION lib)
