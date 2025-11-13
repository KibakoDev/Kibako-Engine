<p align="center">
  <img src="assets/img/KibakoEngine_Logo_Black.png" alt="Kibako Engine Logo" width="180"/>
</p>

<h1 align="center">Kibako Engine</h1>

<p align="center">
  <em>
    A learning-focused C++ engine prototype built from scratch on Direct3D 11.<br>
    Exploring real-time rendering, engine architecture, and tooling from the ground up.
  </em>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/status-early_development-orange" alt="status"/>
  <img src="https://img.shields.io/badge/C%2B%2B-20-blue" alt="language"/>
  <img src="https://img.shields.io/badge/Windows-MSVC%2FVS2022-lightgrey" alt="platform"/>
  <img src="https://img.shields.io/badge/license-MIT-green" alt="license"/>
</p>

---

## 📚 Table of Contents
- [Project Goals](#-project-goals)
- [Current Feature Set](#-current-feature-set)
- [Project Structure](#-project-structure)
- [Build & Run](#-build--run)
- [Architecture Overview](#-architecture-overview)
- [Learning Roadmap](#-learning-roadmap)
- [Known Limitations & Cleanup](#-known-limitations--cleanup)
- [License](#-license)

---

## 🎯 Project Goals
Kibako Engine is a personal sandbox created by a solo developer (19 y/o) to deepen knowledge of modern graphics programming and engine design. The repository documents a journey through:

- Building a render pipeline from scratch using **Direct3D 11**.
- Designing modular systems with clear responsibilities (application loop, rendering, input, profiling, etc.).
- Experimenting with resource management, tooling, and clean code practices.

> ⚠️ The engine is **not production-ready**. APIs will change frequently and many subsystems are still in exploratory stages.

---

## ✨ Current Feature Set
- **SDL-powered application layer** that encapsulates window creation, message pumping, and the main run loop.
- **Layer stack architecture** (`Layer` base class) providing hooks for attach/detach/update/render to organize sandbox or future game logic.
- **Direct3D 11 renderer** with an orthographic 2D camera, swap chain management, and a centralized `SpriteBatch2D` for batched sprite submission.
- **Texture loading** through `stb_image` with optional sRGB handling.
- **Input & timing subsystems** that wrap SDL keyboard/mouse events and high-resolution timers.
- **Instrumentation tooling** including a logger with severity levels and a scoped profiler available in debug builds.
- **Sample sandbox** demonstrating animated sprites, renderer usage, and the existing tooling pipeline.

---

## 🗂 Project Structure
```
Kibako-Engine/
├── Kibako2DEngine/      # Core engine source (application, renderer, utilities)
├── Kibako2DSandbox/     # Example sandbox project showcasing the engine
├── assets/              # Engine branding assets and sample textures
├── KibakoEngine.sln     # Visual Studio 2022 solution
└── README.md            # You are here
```

Key modules inside `Kibako2DEngine`:
- `Core/` – Application bootstrap, layer stack, logging, profiling, input, and timing utilities.
- `Renderer/` – Direct3D 11 renderer, sprite batching, textures, and camera handling.
- `Utils/` – Math helpers and shared utility code (currently minimal).

---

## 🛠 Build & Run
1. **Requirements**
   - Windows 10/11 (x64)
   - Visual Studio 2022 with **MSVC** toolset
   - Desktop development with C++ workload, including DirectX 11 SDK components

2. **Clone the repository**
   ```bash
   git clone https://github.com/KibakoDev/Kibako-Engine.git
   ```

3. **Open the solution**
   - Launch `KibakoEngine.sln` in Visual Studio 2022.
   - Set `Kibako2DSandbox` as the startup project.

4. **Build & run**
   - Build in `Debug` or `Release` (x64 recommended).
   - Run the sandbox to see rotating/animated sprites rendered via the engine.

> 💡 Shader compilation currently happens at runtime through `D3DCompile`. Ensure the Windows 10 SDK is installed to provide the compiler DLLs.

---

## 🧱 Architecture Overview
### Application & Layer System
- `Application` owns the main loop: initializes SDL, creates the D3D11 renderer, dispatches events, steps layers, and handles graceful shutdown.
- Layers derive from `Layer`, receiving lifecycle callbacks (`OnAttach`, `OnDetach`, `OnUpdate`, `OnRender`) to encapsulate gameplay, editor tools, or debugging UI.

### Input & Time
- `Input` listens to SDL keyboard/mouse events, tracking pressed and released states each frame.
- `Time` wraps `SDL_GetPerformanceCounter` to expose `deltaTime` and `totalTime`, supporting frame-independent updates.

### Renderer & Sprite Batch
- `RendererD3D11` configures the device, swap chain, render targets, and manages a shared orthographic `Camera2D`.
- `SpriteBatch2D` sorts draw commands by layer and shader resource view, rebuilds dynamic vertex/index buffers when needed, and issues efficient `DrawIndexed` calls for sprites.
- `Texture2D` loads image data via `stb_image`, creates GPU textures, and exposes SRV handles to the batcher.

### Tooling
- The logging system outputs to both debugger and console with file/line metadata and optional "logic breakpoints" during development.
- Conditional profiling (enabled in debug) aggregates scope timings and flushes them to the logger to highlight CPU hot spots.

---

## 🧭 Learning Roadmap
To push the engine further and align with future goals:

- **Asset Manager** – Centralize resource loading, caching, and lifetime control so textures/shaders are fetched via handles, paving the way for async loading and memory budgeting.
- **Entity-Component System (ECS)** – Replace ad-hoc layer state with a registry of entities and data-oriented components, making it easier to compose gameplay features and scale complexity.
- **Renderer Extensions** – Prepare for 3D work by abstracting pipeline states, adding depth buffering, and planning for multiple render passes (post-process, shadow maps, etc.).
- **Tooling & UI** – Integrate an in-engine debug overlay (e.g., ImGui) to visualize profiler data, logs, and live tweaks.
- **Build Automation** – Introduce CMake or CI workflows to validate builds and share binaries more easily.

---

## 🧹 Known Limitations & Cleanup
- `Utils/Math.h` currently exposes helpers (clamp/lerp/wrap) that are not yet used anywhere; consider removing or integrating them in future features.
- `SpriteBatch2D::SetPixelSnap` is present but unused. Either hook it up to user settings or trim it to keep the public API lean.
- Runtime shader compilation increases startup time and requires the D3D compiler redistributable; migrating to precompiled `.cso` files would improve usability.
- Resource creation is synchronous and lacks caching, which can become a bottleneck as scenes grow.

---

## 📄 License
MIT © 2025 **KibakoDev**

---

<p align="center">
  <sub>Built with curiosity, patience, and a lot of caffeine ☕</sub>
</p>
