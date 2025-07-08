#include<iostream>
#include<glad.h>
#include <khrplatform.h>
#include<GLFW/glfw3.h>
#include <vector>
#include"imgui.h"
#include"imgui_impl_glfw.h"
#include"imgui_impl_opengl3.h"
#include <cmath>


// Sta³a PI
const float PI = 3.14159265358979f;

struct Point {
    float x, y;
};

enum class SegmentType {
    Line,
    Arc
};

struct Segment {
    Point point;
    SegmentType type;
};

std::vector<Segment> polyline;
SegmentType currentType = SegmentType::Line;

void drawArc(ImDrawList* draw_list, Point p1, Point p2, ImU32 color, float thickness) {
    ImVec2 center = ImVec2((p1.x + p2.x) * 0.5f, (p1.y + p2.y) * 0.5f);
    float radius = std::sqrt((p1.x - center.x) * (p1.x - center.x) + (p1.y - center.y) * (p1.y - center.y));

    float startAngle = std::atan2(p1.y - center.y, p1.x - center.x);
    float endAngle = std::atan2(p2.y - center.y, p2.x - center.x);

    if (endAngle < startAngle)
        endAngle += 2.0f * PI;

    draw_list->PathArcTo(center, radius, startAngle, endAngle);
    draw_list->PathStroke(color, false, thickness);
}

void renderPolyline(ImDrawList* draw_list) {
    for (size_t i = 1; i < polyline.size(); ++i) {
        const Segment& prev = polyline[i - 1];
        const Segment& curr = polyline[i];

        ImU32 color = IM_COL32(255, 255, 0, 255);
        float thickness = 2.0f;

        if (curr.type == SegmentType::Line) {
            draw_list->AddLine(ImVec2(prev.point.x, prev.point.y), ImVec2(curr.point.x, curr.point.y), color, thickness);
        }
        else if (curr.type == SegmentType::Arc) {
            drawArc(draw_list, prev.point, curr.point, color, thickness);
        }
    }
}

int WinMain() {
    // Inicjalizacja GLFW
    if (!glfwInit())
        return -1;

    // Konfiguracja OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Polylines with Arcs", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // VSync

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    // Inicjalizacja ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Rysowanie polilinii
        ImGui::Begin("Polyline Editor");

        if (ImGui::BeginPopupContextWindow()) {
            if (ImGui::MenuItem("Tryb: Linia")) currentType = SegmentType::Line;
            if (ImGui::MenuItem("Tryb: £uk")) currentType = SegmentType::Arc;
            if (ImGui::MenuItem("Undo") && !polyline.empty()) polyline.pop_back();
            ImGui::EndPopup();
        }

        ImGui::Text("Kliknij lewym przyciskiem myszy, aby dodawaæ punkty.");
        ImGui::Text("Kliknij prawym przyciskiem, aby otworzyæ menu kontekstowe.");

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
        ImVec2 canvas_size = ImGui::GetContentRegionAvail();

        draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(50, 50, 50, 255));
        draw_list->PushClipRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), true);

        // Obs³uga klikniêæ
        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            ImVec2 mousePos = ImGui::GetMousePos();
            polyline.push_back({ { mousePos.x, mousePos.y }, currentType });
        }

        renderPolyline(draw_list);

        draw_list->PopClipRect();
        ImGui::End();

        // Renderowanie
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Sprz¹tanie
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
