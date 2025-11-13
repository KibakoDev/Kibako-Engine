# Kibako Engine

Kibako Engine is a compact 2D rendering engine built with C++20, SDL2, and Direct3D 11. It powers a small sandbox application used to explore ideas for a pixel-art project in the style of *Astro Void*. The repository focuses purely on engine-level systems—windowing, input, time keeping, sprite batching, and supporting utilities—without any gameplay-specific logic.

## Features

- SDL2 window creation, input handling, and high-resolution timing.
- Direct3D 11 renderer with pixel-perfect 2D camera, swap chain resize handling, and sprite batching tuned for layered textures.
- Texture loading through `stb_image`, with point sampling defaults suitable for pixel art.
- Unified logging, assertions, and profiling helpers designed for Visual Studio debugging sessions.
- Lightweight sandbox demonstrating layered sprites, tinting, and simple motion using the public engine API.

## Repository structure

```
Kibako2DEngine/
├─ include/KibakoEngine/   Public headers for Core, Renderer, and Utils modules
├─ src/                    Engine implementation files
├─ third_party/            stb_image.h (header-only)
└─ Kibako2DEngine.vcxproj  Visual Studio project file

Kibako2DSandbox/
├─ include/                Sandbox layer header
├─ src/                    Sandbox main loop and demo layer
└─ Kibako2DSandbox.vcxproj Visual Studio project file
```

The engine is built as a static library and linked by the sandbox application.

## Build requirements

- Windows 10/11 with a Direct3D 11 capable GPU.
- Visual Studio 2022 (v143 toolset) with the **Desktop development with C++** workload.
- [vcpkg](https://github.com/microsoft/vcpkg) for dependency management (integrated mode recommended).

### Installing dependencies with vcpkg

```powershell
# Clone and bootstrap vcpkg if necessary
git clone https://github.com/microsoft/vcpkg.git C:\dev\vcpkg
C:\dev\vcpkg\bootstrap-vcpkg.bat

# Install SDL2 for both Win32 and x64
C:\dev\vcpkg\vcpkg install sdl2:x86-windows sdl2:x64-windows

# Enable user-wide integration so Visual Studio picks up vcpkg automatically
C:\dev\vcpkg\vcpkg integrate install
```

`stb_image` ships with the repository and does not require installation.

### Building the solution

1. Open `KibakoEngine.sln` in Visual Studio 2022.
2. Select a configuration (`Debug` or `Release`) and platform (`Win32` or `x64`).
3. Build the solution. Both projects treat warnings as errors, compile with `/W4`, and request the Direct3D debug layer when available in debug builds.

## Running the sandbox

Launch the `Kibako2DSandbox` target after a successful build. The sandbox opens a resizable window, loads a single texture, and uses the engine API to batch a few sprites with layered tinting and subtle animation. Press `Esc` to exit.

## Development status

Kibako Engine is an early-stage learning project. The systems are intentionally lean but organized so additional tooling—debug overlays, resource managers, or gameplay layers—can be added as the Astro Void concept evolves.

## License

MIT License © 2025 KibakoDev

