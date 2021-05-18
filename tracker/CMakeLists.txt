project(tracker)

set(HEADER ${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB_RECURSE SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCE})
target_include_directories(${PROJECT_NAME} PUBLIC ${HEADER})
target_link_libraries(${PROJECT_NAME} -static-libgcc -static-libstdc++)
