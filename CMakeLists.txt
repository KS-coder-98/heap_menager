cmake_minimum_required(VERSION 3.14)
project(heap_menager_so2)

set(CMAKE_CXX_STANDARD 14)

add_executable(heap_menager_so2 main.cpp memmanager.cpp custom_unistd.cpp custom_unistd.h)