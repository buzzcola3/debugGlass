load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "imgui_core",
    srcs = [
        "imgui.cpp",
        "imgui_demo.cpp",
        "imgui_draw.cpp",
        "imgui_tables.cpp",
        "imgui_widgets.cpp",
        "misc/cpp/imgui_stdlib.cpp",
    ],
    hdrs = glob([
        "*.h",
        "backends/*.h",
        "misc/cpp/*.h",
    ]),
    includes = [
        ".",
        "backends",
        "misc/cpp",
    ],
    strip_include_prefix = ".",
    textual_hdrs = ["imconfig.h"],
)

cc_library(
    name = "imgui",
    srcs = [
        "backends/imgui_impl_glfw.cpp",
        "backends/imgui_impl_opengl3.cpp",
    ],
    hdrs = [
        "backends/imgui_impl_glfw.h",
        "backends/imgui_impl_opengl3.h",
    ],
    copts = [
        "-DIMGUI_IMPL_OPENGL_LOADER_GLAD",
    ],
    deps = [
        ":imgui_core",
        "@//third_party:glad",
        "@//third_party:glfw",
    ],
    includes = [
        ".",
        "backends",
    ],
    strip_include_prefix = ".",
)
