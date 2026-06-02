#include <imgui.h>
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>
#include <vector>

#include "Grid/Cell.h"
#include "Grid/Grid.h"
#include "Pathfinding/BFS.h"
#include "Pathfinding/DFS.h"
#include "Pathfinding/Dijkstra.h"
#include "Pathfinding/AStar.h"
#include "Maze/RecursiveBacktrack.h"

void drawCell(float x, float y, float size)
{
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + size, y);
    glVertex2f(x + size, y - size);
    glVertex2f(x, y - size);
    glEnd();

    glColor3f(0.15f, 0.15f, 0.15f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + size, y);
    glVertex2f(x + size, y - size);
    glVertex2f(x, y - size);
    glEnd();
}

Cell getCellFromMouse(GLFWwindow* window)
{
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);

    int w, h;
    glfwGetWindowSize(window, &w, &h);

    float x = (float)mx / (float)w;
    float y = (float)my / (float)h;

    float ndcX = x * 2.0f - 1.0f;
    float ndcY = 1.0f - y * 2.0f;

    float cellSizeX = 2.0f / GRID_WIDTH;
    float cellSizeY = 2.0f / GRID_HEIGHT;
    float cellSize = (cellSizeX < cellSizeY) ? cellSizeX : cellSizeY;

    float gridW = cellSize * GRID_WIDTH;
    float gridH = cellSize * GRID_HEIGHT;

    float offsetX = -gridW / 2.0f;
    float offsetY = gridH / 2.0f;

    int gridX = (int)((ndcX - offsetX) / cellSize);
    int gridY = (int)((offsetY - ndcY) / cellSize);

    return Cell{gridX, gridY};
}

int startX = -1, startY = -1;
int endX = -1, endY = -1;

int currentTool = 0; // 0: Wall, 1: Sand (Weight 3), 2: Mud (Weight 5)

int main()
{
    glfwInit();

    GLFWwindow* window = glfwCreateWindow(800, 600, "Pathfinding Visualizer", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int w, int h)
    {
        glViewport(0, 0, w, h);
    });

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    initializeGrid();

    float delay = 0.01f; // This tracks the execution step delay
    bool isPaused = false; // This tracks if the visualizer is frozen
    int currentAlgorithm = 0; // For Pathfinding: 0 for BFS, 1 for DFS
    int currentMazeAlgorithm = 0; // For Maze Generation: 0 for RB
    bool isInstant = false; // Toggle for instant execution

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui Control Panel
        ImGui::Begin("Control Panel");
        ImGui::Text("GLFW + ImGui Visualizer");
        ImGui::Separator();

        // Tool Selector
        ImGui::Text("Editing Tool:");
        const char* tools[] = { "Wall (Infinity)", "Sand (Weight 3)", "Mud (Weight 5)" };
        // Use currentTool variable to track selection
        ImGui::Combo("##ToolCombo", &currentTool, tools, IM_ARRAYSIZE(tools));
        ImGui::Spacing();
        ImGui::Separator();

        // Reset Path + Search nodes
        if (ImGui::Button("Reset Path / Search"))
        {
            bfsStarted = false;
            dfsStarted = false;
            dijkstraStarted = false;
            astarStarted = false;
            mazeGenerationStarted = false;
            foundEnd = false;
            resetSearchState();
        }

        // Clear Entire Grid (Wipes paths AND walls back to empty)
        if (ImGui::Button("Clear Entire Grid"))
        {
            bfsStarted = false;
            dfsStarted = false;
            dijkstraStarted = false;
            astarStarted = false;
            mazeGenerationStarted = false;
            foundEnd = false;
            resetSearchState();
            clearAllWalls();

            // Clear start/end location nodes
            if (startX != -1 && startY != -1) grid[startY][startX].isStart = false;
            if (endX != -1 && endY != -1) grid[endY][endX].isEnd = false;
            startX = -1; startY = -1;
            endX = -1; endY = -1;
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Simulation Controls");

        // Only show the Pause/Resume button if an algorithm or maze is running
        // Maze generation doesn't use foundEND, so it's handled separately
        if (((bfsStarted || dfsStarted || dijkstraStarted || astarStarted) && !foundEnd) || mazeGenerationStarted)
        {
            if (isPaused)
            {
                if (ImGui::Button("Resume Simulation"))
                {
                    isPaused = false;
                }
            }
            else
            {
                if (ImGui::Button("Pause Simulation"))
                {
                    isPaused = true;
                }
            }
        }
        else
        {
            isPaused = false;
        }

        ImGui::Checkbox("Instant Run", &isInstant);

        // Gray out the speed slider if Instant Run is active
        if (isInstant) {
            ImGui::TextDisabled("Speed Slider (Disabled in Instant Mode)");
        } else {
            ImGui::SliderFloat("Step Delay (s)", &delay, 0.001f, 0.2f, "%.3f s");
        }

        ImGui::Spacing();
        ImGui::Separator();

        // Algorithm Selection
        ImGui::Text("Select Algorithm:");
        const char* algorithms[] = { "Breadth-First Search (BFS)", "Depth-First Search (DFS)", "Dijkstra's Algorithm", "A* Search" };

        // Disable changing the algorithm while a simulation is actively running
        if (((bfsStarted || dfsStarted || dijkstraStarted || astarStarted) && !foundEnd) || mazeGenerationStarted) {
            ImGui::TextDisabled("%s", algorithms[currentAlgorithm]);
        } else {
            ImGui::Combo("##AlgoCombo", &currentAlgorithm, algorithms, IM_ARRAYSIZE(algorithms));
        }

        if (ImGui::Button("Generate Path")) {
            if (!mazeGenerationStarted && !bfsStarted && !dfsStarted && !dijkstraStarted && !astarStarted)
            {
                if (startX != -1 && startY != -1 && endX != -1 && endY != -1)
                {
                    resetSearchState();
                    foundEnd = false;

                    if (currentAlgorithm == 0) {
                        startBFS(startX, startY);
                    } else if (currentAlgorithm == 1) {
                        startDFS(startX, startY);
                    } else if (currentAlgorithm == 2) {
                        startDijkstra(startX, startY);
                    } else if (currentAlgorithm == 3) {
                        startAStar(startX, startY);
                    }
                }
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Maze Generation");

        const char* mazeAlgorithms[] = { "Recursive Backtracking" };

        // Disable changing maze algorithm if any process is running
        if (mazeGenerationStarted || ((bfsStarted || dfsStarted || dijkstraStarted || astarStarted) && !foundEnd)) {
            ImGui::TextDisabled("%s", mazeAlgorithms[currentMazeAlgorithm]);
        } else {
            ImGui::Combo("##MazeCombo", &currentMazeAlgorithm, mazeAlgorithms, IM_ARRAYSIZE(mazeAlgorithms));
        }

        if (ImGui::Button("Generate Maze"))
        {
            if (!bfsStarted && !dfsStarted && !dijkstraStarted && !astarStarted && !mazeGenerationStarted)
            {
                // Reset pathfinding visuals
                resetSearchState();

                // BUG FIX: Wipe the grid completely clean of ALL walls and old node flags
                for (int y = 0; y < GRID_HEIGHT; y++) {
                    for (int x = 0; x < GRID_WIDTH; x++) {
                        grid[y][x].isWall = false;
                        grid[y][x].isStart = false; // Clear old start node on the grid cells
                        grid[y][x].isEnd = false;   // Clear old end node on the grid cells
                    }
                }

                // Keep our actual tracking coordinates, or default if missing
                if (startX == -1 || startY == -1) {
                    startX = 1;
                    startY = 1;
                }

                // Align to odd coordinates for the maze math
                if (startX % 2 == 0) startX++;
                if (startY % 2 == 0) startY++;

                // Reinforce the start node state safely on the grid
                grid[startY][startX].isStart = true;

                // Clear the end node coordinates entirely since the maze changes everything
                endX = -1;
                endY = -1;

                // Generates whichever algorithm is selected
                if (currentMazeAlgorithm == 0)
                {
                    initRecursiveBacktrack(startX, startY);
                }
                // Future algorithms will be here:
                // else if (currentMazeAlgorithm == 1) { initPrims(startX, startY); }

                mazeGenerationStarted = true;
            }
        }

        ImGui::End();
        ImGui::Render();

        // Space key starts the pathfinding algorithm
        static bool spacePressedLastFrame = false;
        bool spacePressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

        if (spacePressed && !spacePressedLastFrame)
        {
            // Only start pathfinding if a maze isn't currently generating,
            // pathfinding isn't already running, and both start/end nodes exist
            if (!mazeGenerationStarted && !bfsStarted && !dfsStarted && !dijkstraStarted && !astarStarted)
            {
                if (startX != -1 && startY != -1 && endX != -1 && endY != -1) {
                    // Reset previous paths/visited, but keep walls intact
                    resetSearchState();
                    foundEnd = false;

                    if (currentAlgorithm == 0) {
                        startBFS(startX, startY);
                    } else if (currentAlgorithm == 1) {
                        startDFS(startX, startY);
                    } else if (currentAlgorithm == 2) {
                        startDijkstra(startX, startY);
                    } else if (currentAlgorithm == 3) {
                        startAStar(startX, startY);
                    }
                }
            }
        }
        spacePressedLastFrame = spacePressed;

        // Pathfinding Algorithm Timing Engine
        static double lastStepTime = 0.0;
        double currentTime = glfwGetTime();

        if ((bfsStarted || dfsStarted || dijkstraStarted || astarStarted) && !foundEnd)
        {
            if (isInstant)
            {
                // Loop repeatedly in a single frame until it hits a wall or finds the end
                while ((bfsStarted || dfsStarted || dijkstraStarted || astarStarted) && !foundEnd)
                {
                    if (currentAlgorithm == 0) {
                        bfsStep();
                    } else if (currentAlgorithm == 1) {
                        dfsStep();
                    } else if (currentAlgorithm == 2) {
                        dijkstraStep();
                    } else if (currentAlgorithm == 3) {
                        astarStep();
                    }
                }
            }
            else if (!isPaused)
            {
                // Frame-by-frame delay timer
                if (currentTime - lastStepTime >= (double)delay)
                {
                    if (currentAlgorithm == 0) {
                        bfsStep();
                    } else if (currentAlgorithm == 1) {
                        dfsStep();
                    } else if (currentAlgorithm == 2) {
                        dijkstraStep();
                    } else if (currentAlgorithm == 3) {
                        astarStep();
                    }
                    lastStepTime = currentTime;
                }
            }
        }

        // Maze Generation Timing Engine
        if (mazeGenerationStarted)
        {
            if (isInstant)
            {
                // Loop repeatedly in a single frame until the active algorithm returns true
                bool mazeDone = false;
                while (!mazeDone)
                {
                    if (currentMazeAlgorithm == 0) {
                        mazeDone = recursiveBacktrackStep();
                    }
                }

                // BUG 2 FIX: Reinforce our start node on the grid properties when instant generation wraps up
                grid[startY][startX].isStart = true;
                grid[startY][startX].isWall = false;
                mazeGenerationStarted = false;
            }
            else if (!isPaused)
            {
                // Frame-by-frame delay timer
                if (currentTime - lastStepTime >= (double)delay)
                {
                    bool mazeDone = false;

                    if (currentMazeAlgorithm == 0) {
                        mazeDone = recursiveBacktrackStep();
                    }

                    if (mazeDone)
                    {
                        // BUG 2 FIX: Reinforce our start node on the grid properties when frame-by-frame generation wraps up
                        grid[startY][startX].isStart = true;
                        grid[startY][startX].isWall = false;
                        mazeGenerationStarted = false;
                    }
                    lastStepTime = currentTime;
                }
            }
        }

        // Build path when done
        if (foundEnd && !grid[endY][endX].isPath)
        {
            if (currentAlgorithm == 0) {
                buildBFSPath(endX, endY);
                bfsStarted = false;
            } else if (currentAlgorithm == 1) {
                buildDFSPath(endX, endY);
                dfsStarted = false;
            } else if (currentAlgorithm == 2) {
                buildDijkstraPath(endX, endY);
                dijkstraStarted = false;
            } else if (currentAlgorithm == 3) {
                buildAStarPath(endX, endY);
                astarStarted = false;
            }
        }

        // Mouse Click Logic
        ImGuiIO& io = ImGui::GetIO();

        if (!io.WantCaptureMouse)
        {
            // BUG 3 FIX: Wrap mouse inputs completely. If an algorithm or maze is active, mouse modifications are strictly locked out
            if (!bfsStarted && !dfsStarted && !dijkstraStarted && !astarStarted && !mazeGenerationStarted)
            {
                Cell c = getCellFromMouse(window);
                int x = c.x;
                int y = c.y;

                if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT)
                {
                    // Checks if Shift key is being held down
                    bool shiftPressed = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                                         glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

                    // Left Click: Place Wall node
                    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                        if (!grid[y][x].isStart && !grid[y][x].isEnd) {
                            if (shiftPressed) {
                                // Eraser logic
                                grid[y][x].isWall = false;
                                grid[y][x].isSand = false;
                                grid[y][x].isMud = false;
                                grid[y][x].weight = 1;
                            } else {
                                // Painting logic based on selected tool
                                if (currentTool == 0) { // Wall
                                    grid[y][x].isWall = true;
                                    grid[y][x].isSand = false;
                                    grid[y][x].isMud = false;
                                    grid[y][x].weight = 1; // Walls don't need weight math
                                } else if (currentTool == 1) { // Sand
                                    grid[y][x].isWall = false;
                                    grid[y][x].isSand = true;
                                    grid[y][x].isMud = false;
                                    grid[y][x].weight = 3;
                                } else if (currentTool == 2) { // Mud
                                    grid[y][x].isWall = false;
                                    grid[y][x].isSand = false;
                                    grid[y][x].isMud = true;
                                    grid[y][x].weight = 5;
                                }
                            }
                        }
                    }

                    // Right Click: Place Start node
                    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
                    {
                        if (!grid[y][x].isEnd) {
                            // BUG 2 FIX: Check both the grid AND track our index variables cleanly to prevent ghost nodes
                            if (startX != -1 && startY != -1) {
                                grid[startY][startX].isStart = false;
                            }
                            startX = x; startY = y;
                            grid[y][x].isStart = true;
                            grid[y][x].isWall = false; // Clear wall if placed over one
                        }
                    }

                    // Middle Click: Place End node
                    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
                    {
                        if (!grid[y][x].isStart) {
                            // BUG 2 FIX: Check both the grid AND track our index variables cleanly to prevent ghost nodes
                            if (endX != -1 && endY != -1) {
                                grid[endY][endX].isEnd = false;
                            }
                            endX = x; endY = y;
                            grid[y][x].isEnd = true;
                            grid[y][x].isWall = false; // Clear wall if placed over one
                        }
                    }
                }
            }
        }

        // OpenGL Draw Loop
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        float cellSize = 2.0f / GRID_WIDTH;
        float gridW = cellSize * GRID_WIDTH;
        float gridH = cellSize * GRID_HEIGHT;

        float offsetX = -gridW / 2.0f;
        float offsetY =  gridH / 2.0f;

        for (int y = 0; y < GRID_HEIGHT; y++) {
            for (int x = 0; x < GRID_WIDTH; x++) {
                float screenX = offsetX + x * cellSize;
                float screenY = offsetY - y * cellSize;

                if (grid[y][x].isStart)
                    glColor3f(0.0f, 1.0f, 0.35f); // Green
                else if (grid[y][x].isEnd)
                    glColor3f(1.0f, 0.15f, 0.15f); // Red
                else if (grid[y][x].isPath)
                    glColor3f(1.0f, 0.85f, 0.0f); // Yellow
                else if (grid[y][x].visited)
                    glColor3f(0.2f, 0.4f, 1.0f); // Blue
                else if (grid[y][x].isWall)
                    glColor3f(0.05f, 0.05f, 0.05f); // Black
                else if (grid[y][x].isMud)
                    glColor3f(0.35f, 0.25f, 0.15f); // Brown (Mud)
                else if (grid[y][x].isSand)
                    glColor3f(0.76f, 0.70f, 0.50f); // Tan (Sand)
                else
                    glColor3f(0.9f, 0.9f, 0.9f); // Empty

                drawCell(screenX, screenY, cellSize);
            }
        }

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}