cmake_minimum_required(VERSION 3.6)
add_executable(client main.cpp client_program.cpp client_program.h ../shared.h)
target_link_libraries(client Qt5::Widgets Qt5::Network)
