#include <imgui.h>
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>
#include <vector>

#include "Cell.h"
#include "Grid.h"
#include "BFS.h"
#include "DFS.h"

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
    int currentAlgorithm = 0; // 0 for BFS, 1 for DFS
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

        // Reset Path + Search nodes
        if (ImGui::Button("Reset Path / Search"))
        {
            bfsStarted = false;
            dfsStarted = false;
            foundEnd = false;
            resetSearchState();
        }

        // Clear Entire Grid (Wipes paths AND walls back to empty)
        if (ImGui::Button("Clear Entire Grid"))
        {
            bfsStarted = false;
            dfsStarted = false;
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

        // Only show the Pause/Resume button if an algorithm is running
        if ((bfsStarted || dfsStarted) && !foundEnd)
        {
            // If algorithm is currently paused, show a "Resume" button
            if (isPaused)
            {
                if (ImGui::Button("Resume Simulation"))
                {
                    isPaused = false;
                }
            }
            // If algorithm is running, show a "Pause" button
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
            // Reset isPaused automatically if no simulation is active
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
        const char* algorithms[] = { "Breadth-First Search (BFS)", "Depth-First Search (DFS)" };

        // Disable changing the algorithm while a simulation is actively running
        if (bfsStarted || dfsStarted) {
            ImGui::TextDisabled("%s", algorithms[currentAlgorithm]);
        } else {
            ImGui::Combo("##AlgoCombo", &currentAlgorithm, algorithms, IM_ARRAYSIZE(algorithms));
        }

        ImGui::End();

        ImGui::Render();

        // Space key starts the BFS algorithm
        static bool spacePressedLastFrame = false;
        bool spacePressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

        if (spacePressed && !spacePressedLastFrame)
        {
            if (startX != -1 && startY != -1)
            {
                // Check which algorithm is selected in the dropdown
                if (currentAlgorithm == 0) {
                    startBFS(startX, startY);
                } else if (currentAlgorithm == 1) {
                    startDFS(startX, startY);
                }
            }
        }
        spacePressedLastFrame = spacePressed;

        // Algorithm timing loop
        static double lastStepTime = 0.0;
        double currentTime = glfwGetTime();

        if ((bfsStarted || dfsStarted) && !foundEnd)
        {
            if (isInstant)
            {
                // Loop repeatedly in a single frame until it hits a wall or finds the end
                while ((bfsStarted || dfsStarted) && !foundEnd)
                {
                    if (currentAlgorithm == 0) {
                        bfsStep();
                    } else if (currentAlgorithm == 1) {
                        dfsStep();
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
            } else if (currentAlgorithm == 1) {
                buildDFSPath(endX, endY);
            }
        }

        // Mouse Click Logic
        ImGuiIO& io = ImGui::GetIO();

        if (!io.WantCaptureMouse)
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
                if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
                {
                    if (!grid[y][x].isStart && !grid[y][x].isEnd) {
                        if (shiftPressed) {
                            grid[y][x].isWall = false;  // Shift + Left Click clears walls
                        } else {
                            grid[y][x].isWall = true;   // Hold Left Click paints walls
                        }
                    }
                }

                // Right Click: Place Start node
                if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
                {
                    if (!grid[y][x].isEnd) {
                        if (startX != -1) grid[startY][startX].isStart = false;
                        startX = x; startY = y;
                        grid[y][x].isStart = true;
                        grid[y][x].isWall = false; // Clear wall if placed over one
                    }
                }

                // Middle Click: Place End node
                if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
                {
                    if (!grid[y][x].isStart) {
                        if (endX != -1) grid[endY][endX].isEnd = false;
                        endX = x; endY = y;
                        grid[y][x].isEnd = true;
                        grid[y][x].isWall = false; // Clear wall if placed over one
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

        for (int y = 0; y < GRID_HEIGHT; y++)
        {
            for (int x = 0; x < GRID_WIDTH; x++)
            {
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