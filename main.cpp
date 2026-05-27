#include <imgui.h>
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>
#include <vector>
#include <queue>
#include <cstring>

#include "Cell.h"
#include "Grid.h"

std::queue<std::pair<int,int>> bfsQueue;
bool visited[80][45] = {false};
bool bfsStarted = false;

std::pair<int,int> parent[80][45];
bool foundEnd = false;

double lastStepTime = 0.0;
double bfsDelay = 0.02; // 20ms per step

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
    float offsetY =  gridH / 2.0f;

    int gridX = (int)((ndcX - offsetX) / cellSize);
    int gridY = (int)((offsetY - ndcY) / cellSize);

    return Cell{gridX, gridY};
}

int startX = -1, startY = -1;
int endX = -1, endY = -1;

void startBFS(int sx, int sy)
{
    while (!bfsQueue.empty()) bfsQueue.pop();
    memset(visited, false, sizeof(visited));

    foundEnd = false;

    bfsQueue.push({sx, sy});
    visited[sx][sy] = true;
    grid[sy][sx].visited = true;

    bfsStarted = true;
}

void bfsStep()
{
    if (bfsQueue.empty() || foundEnd) return;

    auto [x, y] = bfsQueue.front();
    bfsQueue.pop();

    int dx[4] = {1, -1, 0, 0};
    int dy[4] = {0, 0, 1, -1};

    for (int i = 0; i < 4; i++)
    {
        int nx = x + dx[i];
        int ny = y + dy[i];

        if (nx < 0 || nx >= GRID_WIDTH || ny < 0 || ny >= GRID_HEIGHT)
            continue;

        if (visited[nx][ny] || grid[ny][nx].isWall)
            continue;

        visited[nx][ny] = true;
        grid[ny][nx].visited = true;

        parent[nx][ny] = {x, y};   // store where we came from
        bfsQueue.push({nx, ny});

        // Stop condition
        if (grid[ny][nx].isEnd)
        {
            foundEnd = true;
            return;
        }
    }
}

void buildPath(int ex, int ey)
{
    int x = ex;
    int y = ey;

    while (!(x == startX && y == startY))
    {
        grid[y][x].isPath = true;

        auto p = parent[x][y];
        x = p.first;
        y = p.second;
    }
}

int main() {
    glfwInit();

    GLFWwindow* window = glfwCreateWindow(800, 600, "Pathfinding Visualizer", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int w, int h) {
        glViewport(0, 0, w, h);
    });

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    initializeGrid();

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Control Panel");
        ImGui::Text("GLFW + ImGui");
        ImGui::End();

        ImGui::Render();

        // Space starts the BFS algorithm
        static bool spacePressedLastFrame = false;
        bool spacePressed = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

        if (spacePressed && !spacePressedLastFrame)
        {
            if (!bfsStarted && startX != -1)
            {
                startBFS(startX, startY);
            }
        }

        spacePressedLastFrame = spacePressed;

        // Auto BFS timing loop
        double currentTime = glfwGetTime();

        if (bfsStarted && !foundEnd)
        {
            if (currentTime - lastStepTime >= bfsDelay)
            {
                bfsStep();
                lastStepTime = currentTime;
            }
        }

        // Build path when done
        if (foundEnd && !grid[endY][endX].isPath)
        {
            buildPath(endX, endY);
        }

        // Mouse click logic
        ImGuiIO& io = ImGui::GetIO();

        if (!io.WantCaptureMouse)
        {
            Cell c = getCellFromMouse(window);
            int x = c.x;
            int y = c.y;

            if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT)
            {
                if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
                    grid[y][x].isWall = !grid[y][x].isWall;

                if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
                {
                    if (startX != -1)
                        grid[startY][startX].isStart = false;

                    startX = x; startY = y;
                    grid[y][x].isStart = true;
                }

                if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
                {
                    if (endX != -1)
                        grid[endY][endX].isEnd = false;

                    endX = x; endY = y;
                    grid[y][x].isEnd = true;
                }
            }
        }

        // Render
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
                    glColor3f(0.0f, 1.0f, 0.35f); // green
                else if (grid[y][x].isEnd)
                    glColor3f(1.0f, 0.15f, 0.15f); // red
                else if (grid[y][x].isPath)
                    glColor3f(1.0f, 0.85f, 0.0f); // yellow
                else if (grid[y][x].visited)
                    glColor3f(0.2f, 0.4f, 1.0f); // blue
                else if (grid[y][x].isWall)
                    glColor3f(0.05f, 0.05f, 0.05f); // black
                else
                    glColor3f(0.9f, 0.9f, 0.9f); // empty

                drawCell(screenX, screenY, cellSize);
            }
        }

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}