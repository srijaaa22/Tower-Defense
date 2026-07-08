cmake_minimum_required(VERSION 3.15)
project(Tower-Defense CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 CONFIG REQUIRED)

add_executable(Tower-Defense src/main.cpp)
target_include_directories(Tower-Defense PRIVATE include)

target_link_libraries(Tower-Defense
    PRIVATE
    SDL2::SDL2
    SDL2::SDL2main
)