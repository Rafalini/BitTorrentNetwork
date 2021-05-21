set(HEADER ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/TrackerClient.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/TrackerServer.cpp
        )

add_library(tracker_lib STATIC ${SOURCE})
target_include_directories(tracker_lib PUBLIC ${HEADER})
target_link_libraries(tracker_lib pthread utils)

add_executable(tracker ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
target_link_libraries(tracker PUBLIC tracker_lib -static-libgcc -static-libstdc++)
