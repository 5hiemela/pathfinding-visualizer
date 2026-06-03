cmake_minimum_required(VERSION 3.20)
project(PathfindingVisualizer)

set(CMAKE_CXX_STANDARD 20)

find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

add_executable(PathfindingVisualizer
        main.cpp

        extern/imgui/imgui.cpp
        extern/imgui/imgui_draw.cpp
        extern/imgui/imgui_widgets.cpp
        extern/imgui/imgui_tables.cpp

        extern/imgui/backends/imgui_impl_glfw.cpp
        extern/imgui/backends/imgui_impl_opengl3.cpp
        Grid/Cell.h
        Grid/Grid.h
        Grid/Grid.cpp
        Pathfinding/BFS.h
        Pathfinding/BFS.cpp
        Pathfinding/DFS.h
        Pathfinding/DFS.cpp
        Maze/RecursiveBacktrack.h
        Maze/RecursiveBacktrack.cpp
        Pathfinding/Dijkstra.h
        Pathfinding/Dijkstra.cpp
        Pathfinding/AStar.cpp
        Pathfinding/AStar.h
        Pathfinding/Bidirectional.cpp
        Pathfinding/Bidirectional.h
        Maze/Prims.cpp
        Maze/Prims.h
        Maze/Kruskals.cpp
        Maze/Kruskals.h
)

target_include_directories(PathfindingVisualizer PRIVATE
        extern/imgui
        extern/imgui/backends
)

target_link_libraries(PathfindingVisualizer PRIVATE
        glfw
        imgui::imgui
        opengl32
)