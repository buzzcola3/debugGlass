#include "debugglass/debugglass.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <imgui.h>

#include <iostream>
#include <mutex>
#include <thread>
#include <utility>

namespace {
constexpr char kGlslVersion[] = "#version 330";
}

namespace debugglass {

DebugGlass::~DebugGlass() {
    Stop();
}

bool DebugGlass::Run(const DebugGlassOptions& options) {
    if (running_.exchange(true)) {
        return false;
    }

    stop_requested_.store(false);

    worker_ = std::thread(&DebugGlass::ThreadMain, this, options);
    return true;
}

void DebugGlass::Stop() {
    stop_requested_.store(true);
    if (worker_.joinable()) {
        worker_.join();
        worker_ = std::thread();
    }
    running_.store(false);
}

bool DebugGlass::IsRunning() const {
    return running_.load();
}

void DebugGlass::SetBackgroundRenderer(BackgroundRenderCallback callback) {
    std::lock_guard<std::mutex> lock(background_mutex_);
    background_callback_ = std::move(callback);
}

void DebugGlass::ThreadMain(DebugGlassOptions options) {
    GLFWwindow* window = nullptr;
    bool glfw_initialized = false;
    bool imgui_created = false;
    bool imgui_glfw_backend = false;
    bool imgui_opengl_backend = false;

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        running_.store(false);
        return;
    }
    glfw_initialized = true;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

#if defined(__APPLE__)
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    window = glfwCreateWindow(options.width, options.height, options.title.c_str(), nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        running_.store(false);
        return;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to load OpenGL functions via GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        running_.store(false);
        return;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    imgui_created = true;
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        std::cerr << "Failed to initialize ImGui GLFW backend" << std::endl;
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
        running_.store(false);
        return;
    }
    imgui_glfw_backend = true;

    if (!ImGui_ImplOpenGL3_Init(kGlslVersion)) {
        std::cerr << "Failed to initialize ImGui OpenGL backend" << std::endl;
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
        running_.store(false);
        return;
    }
    imgui_opengl_backend = true;

    glClearColor(0.1f, 0.3f, 0.6f, 1.0f);

    while (!glfwWindowShouldClose(window) && !stop_requested_.load()) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        const auto windows_snapshot = windows.Snapshot();
        if (windows_snapshot.empty()) {
            ImGui::Begin("DebugGlass");
            ImGui::TextUnformatted("DebugGlass overlay running...");
            ImGui::End();
        } else {
            for (const auto& window : windows_snapshot) {
                if (!window) {
                    continue;
                }
                const std::string& window_name = window->name();
                const char* title = window_name.empty() ? "Window" : window_name.c_str();
                ImGui::Begin(title);
                window->Render();
                ImGui::End();
            }
        }

        ImGui::Render();
        int display_w = 0;
        int display_h = 0;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);

        BackgroundRenderCallback background;
        {
            std::lock_guard<std::mutex> lock(background_mutex_);
            background = background_callback_;
        }
        if (background) {
            background();
        }

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();

        std::this_thread::sleep_for(options.frame_time);
    }

    if (imgui_opengl_backend) {
        ImGui_ImplOpenGL3_Shutdown();
    }
    if (imgui_glfw_backend) {
        ImGui_ImplGlfw_Shutdown();
    }
    if (imgui_created) {
        ImGui::DestroyContext();
    }
    if (window) {
        glfwDestroyWindow(window);
    }
    if (glfw_initialized) {
        glfwTerminate();
    }

    running_.store(false);
}

}  // namespace debugglass
