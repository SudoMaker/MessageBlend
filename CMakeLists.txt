cmake_minimum_required(VERSION 3.4)
project(MessageBlend C)

add_library(xcobs src/xcobs.c)
target_include_directories(xcobs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(msgblend src/msgblend.c)
target_include_directories(msgblend PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(msgblend xcobs)

add_executable(test_linux_pty test/linux_pty.c)
target_link_libraries(test_linux_pty msgblend)