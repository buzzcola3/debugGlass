# DebugGlass

DebugGlass is a Dear ImGui overlay that runs on a dedicated thread so native applications can expose runtime telemetry without blocking their main loop. This repository also documents a cross-platform Bazel + Zig toolchain configuration for hermetic builds on Linux, macOS, and Windows.

## Prerequisites
- Bazel 8+ with Bzlmod enabled (default)
- Python 3 (for Bazel's hermetic_cc_toolchain repo rule)
- Network access for fetching the Zig SDK, GLFW, and the llvm-mingw SDK used for Windows import libraries

## Getting Started
```bash
bazel run //examples:hello_debugglass          # minimal overlay bootstrap
bazel run //examples:subwindow_demo            # graphs, structures, message monitor
bazel run //examples:message_monitor_demo      # standalone message monitor widget
bazel run //examples:background_demo           # custom background rendering
```
`hello_debugglass` opens the overlay, prints a status line, and exits after a short delay. Use it as a skeleton for integrating DebugGlass into other binaries.

## Cross-compiling with Zig
All Zig toolchains from `hermetic_cc_toolchain` are registered, so any supported platform can be targeted:
```bash
# Example: build the GLFW sample for Windows from Linux/macOS
bazel build --platforms=@zig_sdk//platform:windows_amd64 //test:window
```
MinGW import libraries (user32, gdi32, shell32, opengl32) come from the bundled `llvm_mingw_sdk` archive via `third_party/windows_sdk`, enabling cross-linking without a host Windows SDK.

## Project Layout
- `MODULE.bazel` – Bzlmod dependencies (hermetic Zig toolchain, GLFW, llvm-mingw SDK)
- `third_party/` – wrappers for GLFW and platform SDK bits
- `debugglass/` – library sources (`debugglass.h/.cpp`)
- `examples/` – runnable samples (`hello_debugglass`, `subwindow_demo`, `message_monitor_demo`, `background_demo`)

## Rendering Custom Backgrounds
Register a callback to draw behind the overlay before ImGui renders each frame:
```cpp
debugglass::DebugGlass monitor;
monitor.SetBackgroundRenderer([]() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
});
```
Use the callback to upload textures, draw quads, or simply change the clear color. `examples/background_demo` shows the pattern in context.

## Inspecting Build Targets
Use Bazel's query command to list every buildable target in this repo:
```bash
bazel query //...
```
Scope it to a specific package (e.g., `//examples/...`) if you prefer a smaller listing.

## IDE/clangd Support
We vendor [Hedron's Bazel compile-commands extractor](https://github.com/hedronvision/bazel-compile-commands-extractor) so editors like `clangd` can understand the project. After editing BUILD files, regenerate `compile_commands.json` with:
```bash
bazel run //devtools:refresh_compile_commands
```
By default this refreshes the core library plus the sample binaries (`//examples:hello_debugglass`, `//examples:subwindow_demo`). You can tweak the target list inside `devtools/BUILD.bazel` if your workflow needs additional top-level outputs, or fall back to the upstream catch-all command:
```bash
bazel run @hedron_compile_commands//:refresh_all
```
The generated `compile_commands.json` will appear at the workspace root; point `clangd`, `clang-tidy`, or other C/C++ tooling at that file for richer editor integration.
