cmake_minimum_required(VERSION 4.0)
project(PathfindingVisualizer)

set(CMAKE_CXX_STANDARD 20)

find_package(SFML COMPONENTS Graphics Window System REQUIRED)

add_executable(PathfindingVisualizer main.cpp)

target_link_libraries(PathfindingVisualizer PRIVATE
        SFML::Graphics
        SFML::Window
        SFML::System
)