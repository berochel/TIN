cmake_minimum_required(VERSION 3.15)
project(TIN)

set(CMAKE_CXX_STANDARD 14)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Threads REQUIRED)

include_directories(.)

add_executable(client
        ClassDefinitions.h
        moc_predefs.h
        moc_widget.cpp
        peer.cpp
        ui_widget.h
        widget.cpp
        widget.h)

target_link_libraries(client Qt5::Widgets Threads::Threads)

add_executable(tracker tracker.cpp)