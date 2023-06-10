cmake_minimum_required(VERSION 3.20)

project(game-of-life VERSION 0.1.0)

include_directories(include external)

add_subdirectory(external)
add_subdirectory(src)
