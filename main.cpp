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
#include "Pathfinding/Bidirectional.h"
#include "Maze/RecursiveBacktrack.h"
#include "Maze/Prims.h"
#include "Maze/Kruskals.h"

// Performance Tracking Metrics
double totalCalculationTime = 0.0; // Accumulated execution time in milliseconds
int totalSimulationSteps = 0;      // Total iterations/cells processed

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

extern std::pair<int, int> collisionNode;

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

        // Bottom Control Panel
        float panelHeight = 160.0f;

        // Pin to bottom
        ImGui::SetNextWindowPos(ImVec2(0.0f, ImGui::GetIO().DisplaySize.y - panelHeight), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, panelHeight), ImGuiCond_Always);

        // Lock window properties
        ImGuiWindowFlags dashboardFlags = ImGuiWindowFlags_NoTitleBar |
                                          ImGuiWindowFlags_NoMove |
                                          ImGuiWindowFlags_NoResize |
                                          ImGuiWindowFlags_NoCollapse;

        ImGui::Begin("Control Panel", nullptr, dashboardFlags);
        ImGui::Text("Pathfinding & Maze Visualizer");
        ImGui::Separator();

        // Tool Selector
        bool isSimulationRunning = ((bfsStarted || dfsStarted || dijkstraStarted || astarStarted || bidirectionalStarted) && !foundEnd) || mazeGenerationStarted;

        if (isSimulationRunning) {
            ImGui::TextDisabled("Editing Tool: (Disabled while algorithm is running)");
            ImGui::BeginDisabled();
        } else {
            ImGui::Text("Editing Tool:");
        }

        const char* tools[] = { "Wall (Infinity)", "Sand (Weight 3)", "Mud (Weight 5)" };
        ImGui::Combo("##ToolCombo", &currentTool, tools, IM_ARRAYSIZE(tools));

        if (isSimulationRunning) {
            ImGui::EndDisabled();
        }

        ImGui::Spacing();
        ImGui::Separator();

        // Reset Path + Search nodes
        if (ImGui::Button("Reset Path / Search"))
        {
            bfsStarted = false;
            dfsStarted = false;
            dijkstraStarted = false;
            astarStarted = false;
            bidirectionalStarted = false;
            mazeGenerationStarted = false;
            foundEnd = false;
            totalCalculationTime = 0.0;
            totalSimulationSteps = 0;
            resetSearchState();
        }

        // Clear Entire Grid (Wipes paths AND walls back to empty)
        if (ImGui::Button("Clear Entire Grid"))
        {
            bfsStarted = false;
            dfsStarted = false;
            dijkstraStarted = false;
            astarStarted = false;
            bidirectionalStarted = false;
            mazeGenerationStarted = false;
            foundEnd = false;
            totalCalculationTime = 0.0;
            totalSimulationSteps = 0;
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

        if (((bfsStarted || dfsStarted || dijkstraStarted || astarStarted || bidirectionalStarted) && !foundEnd) || mazeGenerationStarted)
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

        if (isInstant) {
            ImGui::TextDisabled("Speed Slider (Disabled in Instant Mode)");
        } else {
            ImGui::SliderFloat("Step Delay (s)", &delay, 0.001f, 0.2f, "%.3f s");
        }

        ImGui::Spacing();
        ImGui::Separator();

        // Algorithm Selection
        ImGui::Text("Select Algorithm:");
        const char* algorithms[] = {
            "Breadth-First Search (BFS)",
            "Depth-First Search (DFS)",
            "Dijkstra's Algorithm",
            "A* Search",
            "Bidirectional BFS"
        };

        if (((bfsStarted || dfsStarted || dijkstraStarted || astarStarted || bidirectionalStarted) && !foundEnd) || mazeGenerationStarted) {
            ImGui::TextDisabled("%s", algorithms[currentAlgorithm]);
        } else {
            if (ImGui::Combo("##AlgoCombo", &currentAlgorithm, algorithms, IM_ARRAYSIZE(algorithms)))
            {
                bfsStarted = false;
                dfsStarted = false;
                dijkstraStarted = false;
                astarStarted = false;
                bidirectionalStarted = false;
                foundEnd = false;
                collisionNode = {-1, -1};
                totalCalculationTime = 0.0;
                totalSimulationSteps = 0;
                resetSearchState();
            }
        }

        if (ImGui::Button("Generate Path")) {
            if (!mazeGenerationStarted)
            {
                if (startX != -1 && startY != -1 && endX != -1 && endY != -1)
                {
                    bfsStarted = false;
                    dfsStarted = false;
                    dijkstraStarted = false;
                    astarStarted = false;
                    bidirectionalStarted = false;
                    foundEnd = false;
                    totalCalculationTime = 0.0;
                    totalSimulationSteps = 0;

                    resetSearchState();

                    if (currentAlgorithm == 0) {
                        startBFS(startX, startY);
                    } else if (currentAlgorithm == 1) {
                        startDFS(startX, startY);
                    } else if (currentAlgorithm == 2) {
                        startDijkstra(startX, startY);
                    } else if (currentAlgorithm == 3) {
                        startAStar(startX, startY);
                    } else if (currentAlgorithm == 4) {
                        startBidirectional(startX, startY, endX, endY);
                    }
                }
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Maze Generation");

        const char* mazeAlgorithms[] = { "Recursive Backtracking", "Prim's Algorithm", "Kruskal's Algorithm" };

        if (mazeGenerationStarted || ((bfsStarted || dfsStarted || dijkstraStarted || astarStarted || bidirectionalStarted) && !foundEnd)) {
            ImGui::TextDisabled("%s", mazeAlgorithms[currentMazeAlgorithm]);
        } else {
            ImGui::Combo("##MazeCombo", &currentMazeAlgorithm, mazeAlgorithms, IM_ARRAYSIZE(mazeAlgorithms));
        }

        if (ImGui::Button("Generate Maze"))
        {
            if (!bfsStarted && !dfsStarted && !dijkstraStarted && !astarStarted && !bidirectionalStarted && !mazeGenerationStarted)
            {
                resetSearchState();
                foundEnd = false;
                totalCalculationTime = 0.0;
                totalSimulationSteps = 0;

                for (int y = 0; y < GRID_HEIGHT; y++) {
                    for (int x = 0; x < GRID_WIDTH; x++) {
                        grid[y][x].isWall = false;
                        grid[y][x].isStart = false;
                        grid[y][x].isEnd = false;
                    }
                }

                if (startX == -1 || startY == -1) {
                    startX = 1;
                    startY = 1;
                }

                if (startX % 2 == 0) startX++;
                if (startY % 2 == 0) startY++;

                grid[startY][startX].isStart = true;
                endX = -1;
                endY = -1;

                if (currentMazeAlgorithm == 0) {
                    initRecursiveBacktrack(startX, startY);
                } else if (currentMazeAlgorithm == 1) {
                    initPrims(startX, startY);
                } else if (currentMazeAlgorithm == 2) {
                    initKruskals(startX, startY);
                }

                mazeGenerationStarted = true;
            }
        }

        ImGui::End();

        // Floating HUD Performance Overlay
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 280.0f, 20.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(260.0f, 95.0f), ImGuiCond_Always);

        ImGuiWindowFlags overlayFlags = ImGuiWindowFlags_NoDecoration |
                                        ImGuiWindowFlags_NoInputs |
                                        ImGuiWindowFlags_AlwaysAutoResize |
                                        ImGuiWindowFlags_NoSavedSettings |
                                        ImGuiWindowFlags_NoFocusOnAppearing |
                                        ImGuiWindowFlags_NoNav;

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.05f, 0.05f, 0.75f));

        if (ImGui::Begin("Performance HUD", nullptr, overlayFlags))
        {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.75f, 1.0f), "PERFORMANCE METRICS");
            ImGui::Separator();

            ImGui::Text("Steps Processed: %d", totalSimulationSteps);
            ImGui::Text("Execution Time:  %.3f ms", totalCalculationTime);
        }
        ImGui::End();
        ImGui::PopStyleColor();

        ImGui::Render();

        // Space key handles algorithm trigger
        static bool spacePressedLastFrame = false;
        bool spacePressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

        if (spacePressed && !spacePressedLastFrame)
        {
            if (!mazeGenerationStarted && !bfsStarted && !dfsStarted && !dijkstraStarted && !astarStarted && !bidirectionalStarted)
            {
                if (startX != -1 && startY != -1 && endX != -1 && endY != -1) {
                    resetSearchState();
                    foundEnd = false;
                    totalCalculationTime = 0.0;
                    totalSimulationSteps = 0;

                    if (currentAlgorithm == 0) {
                        startBFS(startX, startY);
                    } else if (currentAlgorithm == 1) {
                        startDFS(startX, startY);
                    } else if (currentAlgorithm == 2) {
                        startDijkstra(startX, startY);
                    } else if (currentAlgorithm == 3) {
                        startAStar(startX, startY);
                    } else if (currentAlgorithm == 4) {
                        startBidirectional(startX, startY, endX, endY);
                    }
                }
            }
        }
        spacePressedLastFrame = spacePressed;

        // Pathfinding Algorithm Timing Engine
        static double lastStepTime = 0.0;
        double currentTime = glfwGetTime();

        if ((bfsStarted || dfsStarted || dijkstraStarted || astarStarted || bidirectionalStarted) && !foundEnd)
        {
            if (isInstant)
            {
                double startTime = glfwGetTime();
                while ((bfsStarted || dfsStarted || dijkstraStarted || astarStarted || bidirectionalStarted) && !foundEnd)
                {
                    totalSimulationSteps++;
                    if (currentAlgorithm == 0) {
                        bfsStep();
                    } else if (currentAlgorithm == 1) {
                        dfsStep();
                    } else if (currentAlgorithm == 2) {
                        dijkstraStep();
                    } else if (currentAlgorithm == 3) {
                        astarStep();
                    } else if (currentAlgorithm == 4) {
                        bidirectionalStep();
                    }
                }
                totalCalculationTime = (glfwGetTime() - startTime) * 1000.0;
            }
            else if (!isPaused)
            {
                if (currentTime - lastStepTime >= (double)delay)
                {
                    double startTime = glfwGetTime();
                    totalSimulationSteps++;

                    if (currentAlgorithm == 0) {
                        bfsStep();
                    } else if (currentAlgorithm == 1) {
                        dfsStep();
                    } else if (currentAlgorithm == 2) {
                        dijkstraStep();
                    } else if (currentAlgorithm == 3) {
                        astarStep();
                    } else if (currentAlgorithm == 4) {
                        bidirectionalStep();
                    }

                    totalCalculationTime += (glfwGetTime() - startTime) * 1000.0;
                    lastStepTime = currentTime;
                }
            }
        }

        // Maze Generation Timing Engine
        if (mazeGenerationStarted)
        {
            if (isInstant)
            {
                double startTime = glfwGetTime();
                bool mazeDone = false;
                while (!mazeDone)
                {
                    totalSimulationSteps++;
                    if (currentMazeAlgorithm == 0) {
                        mazeDone = recursiveBacktrackStep();
                    } else if (currentMazeAlgorithm == 1) {
                        mazeDone = primsStep();
                    } else if (currentMazeAlgorithm == 2) {
                        mazeDone = kruskalsStep();
                    }
                }
                totalCalculationTime = (glfwGetTime() - startTime) * 1000.0;

                grid[startY][startX].isStart = true;
                grid[startY][startX].isWall = false;
                mazeGenerationStarted = false;
            }
            else if (!isPaused)
            {
                if (currentTime - lastStepTime >= (double)delay)
                {
                    double startTime = glfwGetTime();
                    totalSimulationSteps++;
                    bool mazeDone = false;

                    if (currentMazeAlgorithm == 0) {
                        mazeDone = recursiveBacktrackStep();
                    } else if (currentMazeAlgorithm == 1) {
                        mazeDone = primsStep();
                    } else if (currentMazeAlgorithm == 2) {
                        mazeDone = kruskalsStep();
                    }

                    totalCalculationTime += (glfwGetTime() - startTime) * 1000.0;

                    if (mazeDone)
                    {
                        grid[startY][startX].isStart = true;
                        grid[startY][startX].isWall = false;
                        mazeGenerationStarted = false;
                    }
                    lastStepTime = currentTime;
                }
            }
        }

        // Build path when done
        if (foundEnd)
        {
            if (endX >= 0 && endX < GRID_WIDTH && endY >= 0 && endY < GRID_HEIGHT)
            {
                if (!grid[endY][endX].isPath)
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
                    } else if (currentAlgorithm == 4) {
                        if (collisionNode.first != -1 && collisionNode.second != -1) {
                            buildBidirectionalPath(collisionNode.first, collisionNode.second);
                        }
                        bidirectionalStarted = false;
                    }
                }
            }
            else {
                foundEnd = false;
            }
        }

        // Mouse Click Logic
        ImGuiIO& io = ImGui::GetIO();

        if (!io.WantCaptureMouse)
        {
            if (!bfsStarted && !dfsStarted && !dijkstraStarted && !astarStarted && !bidirectionalStarted && !mazeGenerationStarted)
            {
                Cell c = getCellFromMouse(window);
                int x = c.x;
                int y = c.y;

                if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT)
                {
                    bool shiftPressed = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                                         glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

                    // Left Click: Place Wall/Weight node
                    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                        if (!grid[y][x].isStart && !grid[y][x].isEnd) {
                            if (foundEnd) { foundEnd = false; resetSearchState(); }

                            if (shiftPressed) {
                                grid[y][x].isWall = false;
                                grid[y][x].isSand = false;
                                grid[y][x].isMud = false;
                                grid[y][x].weight = 1;
                            } else {
                                if (currentTool == 0) {
                                    grid[y][x].isWall = true;
                                    grid[y][x].isSand = false;
                                    grid[y][x].isMud = false;
                                    grid[y][x].weight = 1;
                                } else if (currentTool == 1) {
                                    grid[y][x].isWall = false;
                                    grid[y][x].isSand = true;
                                    grid[y][x].isMud = false;
                                    grid[y][x].weight = 3;
                                } else if (currentTool == 2) {
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
                            if (foundEnd) { foundEnd = false; resetSearchState(); }

                            if (startX != -1 && startY != -1) {
                                grid[startY][startX].isStart = false;
                            }
                            startX = x; startY = y;
                            grid[y][x].isStart = true;
                            grid[y][x].isWall = false;
                        }
                    }

                    // Middle Click: Place End node
                    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
                    {
                        if (!grid[y][x].isStart) {
                            if (foundEnd) { foundEnd = false; resetSearchState(); }

                            if (endX != -1 && endY != -1) {
                                grid[endY][endX].isEnd = false;
                            }
                            endX = x; endY = y;
                            grid[y][x].isEnd = true;
                            grid[y][x].isWall = false;
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