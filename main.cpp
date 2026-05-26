#include <imgui.h>
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>
#include <vector>

struct Cell {
    int x, y;
    bool isWall = false;
};

const int GRID_WIDTH = 50;
const int GRID_HEIGHT = 30;

std::vector<std::vector<Cell>> grid;

const float CELL_SIZE = 0.04f;

void drawCell(float x, float y, float size) {

    glBegin(GL_LINE_LOOP);

    glVertex2f(x, y);
    glVertex2f(x + size, y);
    glVertex2f(x + size, y - size);
    glVertex2f(x, y - size);

    glEnd();
}

int main() {
    glfwInit();

    GLFWwindow* window = glfwCreateWindow(800, 600, "Pathfinding Visualizer", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

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

        ImGui::Begin("Control Panel");
        ImGui::Text("GLFW + ImGui is working");
        ImGui::Button("Test");
        ImGui::End();

        ImGui::Render();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        for (int y = 0; y < GRID_HEIGHT; y++) {

            for (int x = 0; x < GRID_WIDTH; x++) {

                float screenX = -1.0f + (x * CELL_SIZE);
                float screenY =  1.0f - (y * CELL_SIZE);

                drawCell(screenX, screenY, CELL_SIZE);
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