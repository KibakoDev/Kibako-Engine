# Kibako Engine: New Game Setup Guide

A step-by-step checklist for creating a **clean, isolated game project** on top of Kibako Engine using Visual Studio and Git. Follow the steps in order the first time; afterwards you can treat this as a repeatable recipe for new games.

## 1) Prerequisites
1. **Windows tooling**: Install **Visual Studio 2022** with the **Desktop development with C++** workload and the **Windows 10 SDK**.
2. **Git**: Ensure Git is installed and configured with your username/email (`git config --global user.name "Your Name"`).
3. **Disk layout**: Pick a workspace path without spaces (e.g., `C:\Dev\`).

## 2) Create your own repository & branch
> Goal: work on a branch that is completely separate from the upstream `main` while keeping a clean history for your game.

1. **Fork or create a new remote**
   - EITHER fork `https://github.com/<org>/Kibako-Engine` on your Git hosting provider, **or** create a brand-new empty repo to host your game (you can still pull updates from upstream later if desired).
2. **Clone**
   ```powershell
   cd C:\Dev
   git clone https://github.com/<your-remote>/Kibako-Engine.git
   cd Kibako-Engine
   ```
3. **Add upstream (optional but recommended)** if you want to pull future engine updates:
   ```powershell
   git remote add upstream https://github.com/<org>/Kibako-Engine.git
   git fetch upstream
   ```
4. **Create a dedicated game branch** (keeps your work isolated from upstream `main`):
   ```powershell
   git checkout -b my-game/base-setup upstream/main   # or origin/main if you skipped upstream
   git push -u origin my-game/base-setup
   ```

## 3) Open the solution in Visual Studio
1. Launch **Visual Studio 2022**.
2. **Open** `KibakoEngine.sln` from your cloned folder. The solution contains two projects:
   - `Kibako2DEngine` — the static library for the engine core.
   - `Kibako2DSandbox` — a sample client that references the engine and shows the expected project wiring. 【KibakoEngine.sln†L1-L41】
3. In the **Solution Configuration** dropdown, pick **Debug** (or **Release**) and **x64**.

## 4) Create your own game client project
> You’ll add a new Visual C++ Application project that links against the engine library, mirroring how `Kibako2DSandbox` is wired.

1. **Add a project**: `File` → `Add` → `New Project…` → **Empty Project (C++)**. Name it `MyGameClient` and place it in `Kibako2DMyGame/` beside the existing projects.
2. **Set the project type**: In **Configuration Properties → General**, ensure **Configuration Type = Application (.exe)** for all configs (the sandbox uses this). 【Kibako2DSandbox/Kibako2DSandbox.vcxproj†L21-L71】
3. **Reference the engine**:
   - Right-click **References** → **Add Reference…** → **Projects** → check **Kibako2DEngine** (the engine builds a static library). 【Kibako2DSandbox/Kibako2DSandbox.vcxproj†L22-L27】
4. **Add include paths** (match the sandbox): **Configuration Properties → C/C++ → General → Additional Include Directories**. Add:
   ```
   $(ProjectDir)include
   $(SolutionDir)Kibako2DEngine\include
   $(SolutionDir)Kibako2DEngine\third_party
   $(SolutionDir)Kibako2DEngine\third_party\imgui
   $(SolutionDir)Kibako2DEngine\third_party\imgui\backends
   ```
   These mirror the sandbox defaults so you can include `KibakoEngine/...` headers immediately. 【Kibako2DSandbox/Kibako2DSandbox.vcxproj†L96-L149】
5. **Add source folders**: Create `include/` and `src/` directories under your new project. Add starter files (e.g., `src/main.cpp`, `include/GameLayer.h`) similar to the sandbox layout so you can bootstrap quickly.
6. **Copy the entry point as a starting template** (optional): use `Kibako2DSandbox/src/main.cpp` and `Kibako2DSandbox/src/GameLayer.cpp` as a reference for how to spin up the engine loop, register layers, and load textures. Keep namespaces and includes aligned with the include paths above. 【Kibako2DSandbox/Kibako2DSandbox.vcxproj†L18-L35】
7. **Assets**: Point to your own asset folder (e.g., `Kibako2DMyGame/assets/`). Add it to the project (Right-click → Add → Existing Item) to ensure build steps copy assets as needed.
8. **Set startup project**: Right-click **MyGameClient** → **Set as Startup Project**. Verify **x64** + **Debug/Release** are selected.

## 5) Verify the build
1. Press **Ctrl+Shift+B** to build the solution. Because the engine is referenced, Visual Studio builds `Kibako2DEngine` first (static library) and then your client.
2. Run (F5) and confirm a window opens. If you reused the sandbox entry point, you should see sprites rendered by the engine.

## 6) Version control workflow for your game
1. **Commit the base setup**
   ```powershell
   git status
   git add Kibako2DMyGame KibakoEngine.sln
   git commit -m "Add MyGame client project wired to Kibako Engine"
   git push
   ```
2. **Iterate safely**: create feature branches off `my-game/base-setup` (e.g., `my-game/player-movement`) and open pull requests into your game repo.
3. **Syncing upstream engine updates (optional)**
   ```powershell
   git checkout main
   git pull upstream main
   git checkout my-game/base-setup
   git merge main   # or rebase if preferred
   ```

## 7) Keep the branch clean
- Avoid editing engine headers/sources in your game branch unless you intend to fork the engine. Prefer engine changes in a separate branch that you can merge selectively.
- Keep your client project self-contained (its own `include/`, `src/`, `assets/` folders) so the solution stays organized.
- Use `.gitignore` for any generated output (VS keeps one at repo root).

With these steps you’ll have a clean, reproducible baseline: your own branch, a custom client project wired to the Kibako Engine static library, and a workflow that lets you pull engine updates without contaminating your game history.
