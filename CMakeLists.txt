cmake_minimum_required(VERSION 3.22)
project(minidb C)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")

set(CMAKE_C_STANDARD 11)

add_executable(minidb src/table.c src/test.c src/utils.c src/utils.h src/parse.c src/parse.h src/reactor.c src/reactor.h)
