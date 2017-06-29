cmake_minimum_required(VERSION 3.6)
add_executable(server ./main.cpp ./server_program.cpp ./server_program.h ../shared.h)
target_link_libraries(server Qt5::Widgets Qt5::Network)
