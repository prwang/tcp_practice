cmake_minimum_required(VERSION 3.6)
add_executable(client main.cpp mainwindow.cpp mainwindow.h ../shared.h)
target_link_libraries(client Qt5::Widgets Qt5::Network)
