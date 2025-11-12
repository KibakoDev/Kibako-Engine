<p align="center"><img src="assets/img/KibakoEngine_Logo_Black.png" alt="Kibako Engine Logo" width="180"/></p>

<h1 align="center">Kibako Engine</h1>

<p align="center">
    <em>Minimal 2D Direct3D 11 sandbox ready to host a pixel-art game.</em>
</p>

---

## Highlights

- **Purpose-built for Astro Void** pre-production: modular C++20 core without any gameplay logic.
- **Deterministic 2D renderer** featuring pixel snapping, point sampling, and texture/layer sprite batching.
- **SDL2 windowing & input**, Direct3D 11 renderer bootstrap, stb_image texture loading.
- **Strict debug tooling**: color-coded logging, assert/HRESULT guards, optional monochrome preview.
- **Zero-dependency build** beyond `SDL2` and `stb_image` (provided through vcpkg).

---

## Engine layout

```
Kibako2DEngine/
├─ include/KibakoEngine/
│  ├─ Core/        (Application, Input, Time, Debug, Log)
│  ├─ Renderer/    (RendererD3D11, Camera2D, SpriteBatch2D, Texture2D, SpriteTypes)
│  └─ Utils/       (Math helpers)
├─ src/            (matching implementation files)
├─ third_party/    (stb_image.h)
└─ Kibako2DEngine.vcxproj

Kibako2DSandbox/
├─ src/main.cpp    (neutral rendering demo)
└─ Kibako2DSandbox.vcxproj
```

The engine project builds a static library. The sandbox links against it and demonstrates a few sprites using different layers, tints, and sampling modes.

---

## Prerequisites

- **Visual Studio 2022** (v143 toolset) with Desktop development for C++ workload.
- **vcpkg** for dependency management (integrated mode recommended).
- Windows 10/11 with the Direct3D 11 runtime.

### Install dependencies via vcpkg

```powershell
# Clone vcpkg if you do not have it yet
git clone https://github.com/microsoft/vcpkg.git C:\dev\vcpkg
C:\dev\vcpkg\bootstrap-vcpkg.bat

# Install required libraries for both Win32 and x64
C:\dev\vcpkg\vcpkg install sdl2:x64-windows sdl2:x86-windows
```

Add the following user-wide integration once:

```powershell
C:\dev\vcpkg\vcpkg integrate install
```

Visual Studio will now pick up SDL2 automatically when you open `KibakoEngine.sln`.

`stb_image` is shipped in `third_party/` and does not require an external package.

---

## Building

1. Open `KibakoEngine.sln` in Visual Studio 2022.
2. Select the desired configuration (`Debug`/`Release`) and platform (`Win32`/`x64`).
3. Build the solution. The engine and sandbox share the same warning level (`/W4`), treat warnings as errors, enforce `/permissive-`, and set `/Zc:__cplusplus`.

The Direct3D debug layer is enabled automatically in debug builds (when present) and the swapchain resizes when the window changes size.

---

## Sandbox controls

| Key | Action |
| --- | ------ |
| `Esc` | Quit the sandbox |
| `M` | Toggle monochrome preview (0 ↔ 1) |
| `P` | Toggle point vs. linear sampling |

The sandbox renders three layered sprites using the same texture with different tints and rotation, demonstrating:

- Pixel snapping (disabled automatically when rotating).
- Stable batching by layer and texture (one draw call per texture per layer).
- Per-frame toggles for sampling and monochrome.

---

## Next steps

- Hook the sandbox to the future Astro Void gameplay module.
- Extend the debug overlay (draw calls, frame time) if needed.
- Add texture atlas helpers and resource hot-reload when the production pipeline requires it.

Until then, Kibako Engine remains a compact, production-ready rendering shell for a pixel-art game.

---

## License

MIT © 2025 KibakoDev

