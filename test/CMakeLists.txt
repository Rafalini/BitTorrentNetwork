project(bittorrent_test)

find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

set(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/tests_main.cpp)
add_subdirectory(config)

add_executable(${PROJECT_NAME} ${SOURCE})
target_link_libraries(${PROJECT_NAME} PUBLIC
        utils
        peer_lib
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        )

add_test(
        NAME ${PROJECT_NAME}
        COMMAND ${PROJECT_NAME}
)
