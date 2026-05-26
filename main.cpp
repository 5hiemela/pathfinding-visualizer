#include <imgui.h>
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>
#include <vector>

struct Cell {
    int x, y;
    bool isWall = false;
};

const int GRID_WIDTH = 80;
const int GRID_HEIGHT = 45;

std::vector<std::vector<Cell>> grid;

void drawCell(float x, float y, float size) {
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

        if (!io.WantCaptureMouse &&
            glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            Cell c = getCellFromMouse(window);

            if (c.x >= 0 && c.x < GRID_WIDTH &&
                c.y >= 0 && c.y < GRID_HEIGHT)
            {
                grid[c.y][c.x].isWall = !grid[c.y][c.x].isWall;
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

                if (grid[y][x].isWall) {
                    glColor3f(0.05f, 0.05f, 0.05f); // dark wall
                } else {
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