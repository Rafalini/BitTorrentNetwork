project(peer)

set(HEADER ${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB_RECURSE SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(peer_lib STATIC ${SOURCE})
target_include_directories(peer_lib PUBLIC ${HEADER})
target_link_libraries(peer_lib pthread tracker_lib)

add_executable(${PROJECT_NAME} ${SOURCE})

set_target_properties( ${PROJECT_NAME}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        )

target_include_directories(${PROJECT_NAME} PUBLIC ${HEADER})
target_link_libraries(${PROJECT_NAME} PUBLIC peer_lib -static-libgcc -static-libstdc++)
