# DebugGlass

DebugGlass is a drop-in Dear ImGui overlay that boots in its own thread so you can instrument any native app quickly. The repository also doubles as a cross-platform Bazel + Zig toolchain playground, demonstrating hermetic builds for Linux/macOS/Windows without relying on host compilers.

## Prerequisites
- Bazel 8+ with Bzlmod enabled (default)
- Python 3 (for Bazel's hermetic_cc_toolchain repo rule)
- Network access for fetching the Zig SDK, GLFW, and the llvm-mingw SDK used for Windows import libraries

## Getting Started
```bash
# Run the "Hello DebugGlass" demo locally (opens a window for ~10 seconds)
bazel run //examples:hello_debugglass

# Launch the richer subwindow/telemetry demo
bazel run //examples:subwindow_demo
```
The example spins up the DebugGlass overlay, prints "Hello World" to stdout, and exits after a short delay. Use it as a template for wiring the library into your own project.

## Cross-compiling with Zig
We register all Zig toolchains from `hermetic_cc_toolchain`, so you can target any supported platform:
```bash
# Example: build the GLFW sample for Windows from Linux/macOS
bazel build --platforms=@zig_sdk//platform:windows_amd64 //test:window
```
MinGW import libraries (user32, gdi32, shell32, opengl32) come from the bundled `llvm_mingw_sdk` archive and are wired in through `third_party/windows_sdk` so cross-linking succeeds without a local Windows SDK.

## Project Layout
- `MODULE.bazel` – Bzlmod dependencies (hermetic Zig toolchain, GLFW, llvm-mingw SDK)
- `third_party/` – wrappers for GLFW and platform SDK bits
- `debugglass/` – library sources (`debugglass.h/.cpp`)
- `examples/` – runnable samples (currently `hello_debugglass`)

Add more DebugGlass-powered examples under `examples/` as new milestones land (custom widgets, telemetry panels, etc.).

## Inspecting Build Targets
Use Bazel's query command to list every buildable target in this repo:
```bash
bazel query //...
```
Scope it to a specific package (e.g., `//examples/...`) if you prefer a smaller listing.

## IDE/clangd Support
We vendor [Hedron's Bazel compile-commands extractor](https://github.com/hedronvision/bazel-compile-commands-extractor) so editors like `clangd` can understand the project. After editing BUILD files, regenerate `compile_commands.json` with:
```bash
bazel run :refresh_compile_commands
```
By default this refreshes the core library plus the sample binaries (`//examples:hello_debugglass`, `//examples:subwindow_demo`). You can tweak the target list inside `BUILD.bazel` if your workflow needs additional top-level outputs, or fall back to the upstream catch-all command:
```bash
bazel run @hedron_compile_commands//:refresh_all
```
The generated `compile_commands.json` will appear at the workspace root; point `clangd`, `clang-tidy`, or other C/C++ tooling at that file for richer editor integration.
