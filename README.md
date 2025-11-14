<p align="center">
  <img src="assets/img/KibakoEngine_Logo_Black.png" alt="Kibako Engine Logo" width="180"/>
</p>

<h1 align="center">Kibako Engine</h1>

<p align="center">
  <em>Early-stage C++ / Direct3D 11 engine for building 2D games on Windows.</em>
</p>

---

## Snapshot
- 🚧 Prototype status: expect rough edges while core systems come together.
- 🎮 Focused on a sprite-based 2D pipeline with an orthographic camera.
- 🧰 Sandbox client renders animated sprites to showcase the engine loop.

## Highlights
- SDL-powered application layer with input, timing, and a lightweight layer stack.
- Direct3D 11 renderer handling textured quads, sprite batching, and camera control.
- Logging and profiling utilities to inspect frame timing during iteration.

## Project Layout
```
Kibako-Engine/
├── Kibako2DEngine/   # Engine sources
├── Kibako2DSandbox/  # Example client
├── assets/           # Branding & sample textures
└── KibakoEngine.sln  # Visual Studio solution
```

## Quick Start (Windows)
1. Install Visual Studio 2022 with the **Desktop development with C++** workload and the Windows 10 SDK.
2. Clone the repo and open `KibakoEngine.sln`.
3. Set `Kibako2DSandbox` as the startup project, choose x64 Debug/Release, then build and run.

## License
MIT © 2025 KibakoDev
