#include <cstdio>
#include <cstdlib>
#include <optional>
#include <string>

#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl2.h"
#include "res/res.h"
#include "world/level.h"
#ifdef REAPER_NAVIGRAPH_EDITOR
#include "ai/navigation_graph.h"
#endif

#ifndef REAPER_EDITOR_TITLE
#define REAPER_EDITOR_TITLE "Reaper3D Editor"
#endif

namespace {
void glfw_error(int code, const char *description)
{
    std::fprintf(stderr, "GLFW error %d: %s\n", code, description);
}
}

int main()
{
    // Resource identifiers are resolved relative to the data directory.  The
    // game and CTest use the same convention, so the editor works from the
    // repository root as well as from build/.
    reaper::res::add_datapath("data");
    reaper::res::add_datapath("../data");

    glfwSetErrorCallback(glfw_error);
    if (!glfwInit())
        return EXIT_FAILURE;

    // The existing level and navigation renderers use the fixed-function
    // pipeline, so the OpenGL2 ImGui backend is the most compatible choice.
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    GLFWwindow *window = glfwCreateWindow(1280, 720, REAPER_EDITOR_TITLE, nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    bool show_demo = false;
    bool running = true;
    char document[256] = "test_level";
#ifdef REAPER_NAVIGRAPH_EDITOR
    char graph_document[256] = "navigation_graph";
#endif
    float grid_size = 10.0f;
    std::optional<reaper::world::LevelInfo> level;
#ifdef REAPER_NAVIGRAPH_EDITOR
    reaper::ai::navigation_graph::Graph graph;
#endif
    std::string status = "No document loaded";
    float camera_zoom = 1.0f;
    float camera_angle = 0.0f;
    while (running && !glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::TextUnformatted(REAPER_EDITOR_TITLE);
        ImGui::Separator();
        ImGui::InputText("Document", document, sizeof(document));
#ifdef REAPER_NAVIGRAPH_EDITOR
        ImGui::InputText("Graph resource", graph_document, sizeof(graph_document));
        if (ImGui::Button("Load graph")) {
            graph.build_graph(graph_document);
            status = "Loaded navigation graph " + std::string(graph_document);
        }
        ImGui::SameLine();
#endif
        if (ImGui::Button("Load")) {
            try {
                level.emplace(document);
#ifdef REAPER_NAVIGRAPH_EDITOR
                graph.build_graph(document);
                status = "Loaded " + std::string(document) + " navigation data";
#else
                status = "Loaded " + std::string(document);
#endif
            } catch (const std::exception &e) {
                level.reset();
                status = std::string("Load failed: ") + e.what();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Save")) {
            if (!level) {
                status = "Load a document before saving";
            } else {
                try {
                    level->save(document);
                    status = "Saved " + std::string(document);
                } catch (const std::exception &e) {
                    status = std::string("Save failed: ") + e.what();
                }
            }
        }
        ImGui::TextWrapped("%s", status.c_str());
        if (level) {
            ImGui::Separator();
            ImGui::Text("Terrain: %s", level->terrain_mesh.c_str());
            ImGui::Text("Objects: %zu groups", level->objectgroups.size());
            ImGui::Text("Scenario: %s", level->scenario.c_str());
            ImGui::InputFloat("Sky altitude", &level->sky_altitude);
            ImGui::InputFloat("Sky width", &level->sky_width);
            ImGui::InputFloat("Fog length", &level->fog_length);
            ImGui::InputFloat("Fog density", &level->fog_density);
        }
        ImGui::SliderFloat("Grid size", &grid_size, 1.0f, 100.0f, "%.1f");
        ImGui::SliderFloat("Zoom", &camera_zoom, 0.25f, 4.0f, "%.2fx");
        ImGui::SliderFloat("Angle", &camera_angle, -180.0f, 180.0f, "%.0f deg");
        ImGui::Checkbox("ImGui demo", &show_demo);
        if (ImGui::Button("Exit"))
            running = false;
        ImGui::TextWrapped("GLFW/ImGui editor. Object and navigation editing remains under migration.");
        ImGui::End();
        if (show_demo)
            ImGui::ShowDemoWindow(&show_demo);

        ImGui::Render();
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(0.07f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        const float extent = 500.0f / camera_zoom;
        glOrtho(-extent, extent, -extent, extent, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotatef(camera_angle, 0.0f, 0.0f, 1.0f);
        glColor3f(0.20f, 0.23f, 0.28f);
        glBegin(GL_LINES);
        for (float p = -extent; p <= extent; p += grid_size) {
            glVertex2f(p, -extent); glVertex2f(p, extent);
            glVertex2f(-extent, p); glVertex2f(extent, p);
        }
        glEnd();
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glColor3f(0.75f, 0.20f, 0.20f); glVertex2f(-extent, 0); glVertex2f(extent, 0);
        glColor3f(0.20f, 0.75f, 0.30f); glVertex2f(0, -extent); glVertex2f(0, extent);
        glEnd();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
