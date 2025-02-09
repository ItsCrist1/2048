cmake_minimum_required(VERSION 3.20)
project(2048)

set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(2048 
    src/main.cpp 
    src/game.cpp 
    src/utils.cpp
)