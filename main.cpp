#include <imgui.h>
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>
#include <vector>

struct Cell {
    int x, y;
    bool isWall = false;
    bool isStart = false;
    bool isEnd = false;
};

const int GRID_WIDTH = 80;
const int GRID_HEIGHT = 45;

std::vector<std::vector<Cell>> grid;

void drawCell(float x, float y, float size)
{
    // fill
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + size, y);
    glVertex2f(x + size, y - size);
    glVertex2f(x, y - size);
    glEnd();

    // outline
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

    // convert to normalized screen space (0 → 1)
    float x = (float)mx / (float)w;
    float y = (float)my / (float)h;

    // convert to NDC (-1 → 1)
    float ndcX = x * 2.0f - 1.0f;
    float ndcY = 1.0f - y * 2.0f;

    float cellSizeX = 2.0f / GRID_WIDTH;
    float cellSizeY = 2.0f / GRID_HEIGHT;
    float cellSize = (cellSizeX < cellSizeY) ? cellSizeX : cellSizeY;

    float gridW = cellSize * GRID_WIDTH;
    float gridH = cellSize * GRID_HEIGHT;

    float offsetX = -gridW / 2.0f;
    float offsetY =  gridH / 2.0f;

    // convert NDC to grid index
    int gridX = (int)((ndcX - offsetX) / cellSize);
    int gridY = (int)((offsetY - ndcY) / cellSize);

    return Cell{gridX, gridY, false};
}

int startX = -1, startY = -1;
int endX = -1, endY = -1;

int main() {
    glfwInit();

    GLFWwindow* window = glfwCreateWindow(800, 600, "Pathfinding Visualizer", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Keep viewport synced with window size
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int w, int h) {
        glViewport(0, 0, w, h);
    });

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    grid.resize(GRID_HEIGHT);
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            grid[y].push_back(Cell{x, y, false});
        }
    }

    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // UI
        ImGui::Begin("Control Panel");
        ImGui::Text("GLFW + ImGui is working");
        ImGui::Button("Test");
        ImGui::End();

        ImGui::Render();

        // Mouse click logic
        ImGuiIO& io = ImGui::GetIO();

        if (!io.WantCaptureMouse)
        {
            Cell c = getCellFromMouse(window);
            int x = c.x;
            int y = c.y;

            if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT)
            {
                // LEFT CLICK - WALL TOGGLE
                if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
                {
                    grid[y][x].isWall = !grid[y][x].isWall;
                }

                // RIGHT CLICK - START NODE
                if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
                {
                    // clear old start
                    if (startX != -1)
                        grid[startY][startX].isStart = false;

                    startX = x;
                    startY = y;

                    grid[y][x].isStart = true;
                }

                // MIDDLE CLICK - END NODE
                if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
                {
                    // clear old end
                    if (endX != -1)
                        grid[endY][endX].isEnd = false;

                    endX = x;
                    endY = y;

                    grid[y][x].isEnd = true;
                }
            }
        }

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Get current window size
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        float aspect = (float)width / (float)height;

        // Base cell size in NDC space
        float cellSizeX = 2.0f / GRID_WIDTH;
        float cellSizeY = 2.0f / GRID_HEIGHT;

        // Keep squares
        float cellSize = (cellSizeX < cellSizeY) ? cellSizeX : cellSizeY;

        // Grid size in NDC
        float gridW = cellSize * GRID_WIDTH;
        float gridH = cellSize * GRID_HEIGHT;

        // Centered origin
        float offsetX = -gridW / 2.0f;
        float offsetY =  gridH / 2.0f;

        // Render grid
        for (int y = 0; y < GRID_HEIGHT; y++) {
            for (int x = 0; x < GRID_WIDTH; x++) {

                float screenX = offsetX + x * cellSize;
                float screenY = offsetY - y * cellSize;

                if (grid[y][x].isStart)
                {
                    glColor3f(0.0f, 1.0f, 0.0f); // green
                }
                else if (grid[y][x].isEnd)
                {
                    glColor3f(1.0f, 0.0f, 0.0f); // red
                }
                else if (grid[y][x].isWall)
                {
                    glColor3f(0.05f, 0.05f, 0.05f); // dark wall
                }
                else
                {
                    glColor3f(0.9f, 0.9f, 0.9f); // empty cell
                }

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