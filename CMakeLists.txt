cmake_minimum_required(VERSION 3.16)
project(bittorrent)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall)

add_subdirectory(utils)
add_subdirectory(peer)
add_subdirectory(tracker)
enable_testing()
add_subdirectory(test)
