cmake_minimum_required(VERSION 3.6)
project(tcp_practice)
find_package(Boost REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)



set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG -fstack-protector-all)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
add_subdirectory(client)
add_subdirectory(server)
