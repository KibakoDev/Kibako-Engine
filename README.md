<p align="center">
  <img src="assets/img/KibakoEngine_Logo_Black.png" alt="Kibako Engine Logo" width="180"/>
</p>

<h1 align="center">Kibako Engine</h1>

<p align="center">
  <em>
    Early-stage 2D game engine prototype built from scratch in C++ with Direct3D&nbsp;11.
  </em>
</p>

<p align="center">
  <img src="https://img.shields.io/badge/status-early_proto-orange" alt="status"/>
  <img src="https://img.shields.io/badge/focus-2D_rendering-blue" alt="focus"/>
  <img src="https://img.shields.io/badge/Windows-MSVC%2FVS2022-lightgrey" alt="platform"/>
  <img src="https://img.shields.io/badge/license-MIT-green" alt="license"/>
</p>

---

## 🚧 Current Status
> ⚠️ The engine is a fragile prototype. Expect missing features, rough edges, and work-in-progress systems.

What currently exists:
- SDL-based application layer that boots a window, polls events, and runs a main loop.
- Layer stack to organise experimental gameplay code in the sandbox project.
- Direct3D&nbsp;11 renderer with a simple orthographic camera and a sprite batch for textured quads.
- Basic input and timing helpers built on SDL.
- Logging and lightweight profiling utilities to inspect behaviour while iterating.
- `Kibako2DSandbox` project that renders animated sprites to validate the pipeline.

What **does not** exist yet:
- Asset management, audio, physics, UI, or scene management.
- Production-ready APIs, documentation, or tooling.
- Stable performance targets (buffer growth, shader compilation, etc.).

---

## 🗺️ Vision & Roadmap
### Near-term (2D focus)
- Harden the sprite renderer: texture atlases, improved batching, precompiled shaders.
- Build core engine services: asset manager, input abstraction, scene/entity organisation.
- Improve developer tools: debug overlay (ImGui), live reload for shaders/assets, better logging outputs.

### Long-term (future 3D ambitions)
- Introduce a flexible renderer architecture (pipelines, render passes, GPU resource management).
- Expand the camera system to support perspective projections and 3D transforms.
- Explore modern rendering techniques and cross-platform backends once the 2D layer feels mature.

---

## 🗂 Project Layout
```
Kibako-Engine/
├── Kibako2DEngine/      # Core engine sources (application, renderer, utilities)
├── Kibako2DSandbox/     # Example sandbox that drives the engine each run
├── assets/              # Branding assets and sample textures
├── KibakoEngine.sln     # Visual Studio 2022 solution
└── README.md            # Project overview
```

Key engine modules today:
- `Core/` – Application bootstrap, layer management, logging, profiling, input, timing.
- `Renderer/` – Direct3D&nbsp;11 device, swap chain, sprite batch, textures, camera.
- `Utils/` – Shared helpers that will grow with the project.

---

## 🛠 Getting Started (Windows)
1. **Install prerequisites**
   - Windows 10/11 (x64)
   - Visual Studio 2022 with the **Desktop development with C++** workload
   - Windows 10 SDK (provides the Direct3D compiler DLLs)

2. **Clone the repository**
   ```bash
   git clone https://github.com/KibakoDev/Kibako-Engine.git
   ```

3. **Open the solution**
   - Launch `KibakoEngine.sln` in Visual Studio 2022.
   - Set `Kibako2DSandbox` as the startup project.

4. **Build & run**
   - Select `Debug` or `Release` (x64).
   - Hit run to see the current sprite test scene.

> 💡 Shaders compile at runtime via `D3DCompile`. Startup will fail if the compiler DLLs are missing.

---

## 🧹 Known Gaps & Cleanup Targets
- Runtime shader compilation is temporary—precompiled `.cso` files are planned.
- Resource creation is synchronous and lacks caching or streaming.

These notes help track what to revisit as the 2D pipeline evolves.

---

## 📄 License
MIT © 2025 **KibakoDev**

---
