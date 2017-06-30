cmake_minimum_required(VERSION 3.6)
add_executable(client main.cpp client_program.cpp client_program.h ../shared.h
        friend.cpp friend.h client_program_ui.cpp)
target_link_libraries(client Qt5::Widgets Qt5::Network)
