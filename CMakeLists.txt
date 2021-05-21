cmake_minimum_required(VERSION 3.17)
project(bittorrent)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(utils)
add_subdirectory(peer)
add_subdirectory(tracker)
add_subdirectory(test)
