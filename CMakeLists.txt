cmake_minimum_required(VERSION 3.31)
project(word_count_stepwise_optimization LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
add_compile_options(-Wall -Wextra -O3 -march=native)

add_subdirectory(extlibs)
add_subdirectory(wordcount)
add_subdirectory(unit-test)
