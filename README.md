# Project: Through Arcane Eyes (UE 5.7)
"Through Arcane Eyes" is a technical vertical slice developed in Unreal Engine 5.7. It serves as a portfolio demonstration of high-level C++ architecture, modern UI patterns (MVVM + Common UI), and advanced shader techniques.

The project features a procedural cube-grid environment where the core gameplay loop revolves around a "Spectral Shift" mechanic—toggling a magical vision mode to reveal hidden geometry and navigate a fractured world.

# 🛠 Technology Stack
* Engine: Unreal Engine 5.7 (C++ 20)
* Input: Enhanced Input System (Modular Mapping Contexts)
* UI Architecture: Common UI (Input Routing) + UMG Viewmodel (Data Binding)
* Rendering: Post-Process Materials, Global Distance Fields, Stencil Buffers, Substrate Materials
* Gameplay: GAS (Gameplay Ability System) — Arcane toggle, Mana attribute, Gameplay Tags
* Animation: Motion Matching (PoseSearch) — realistic locomotion + Arcane float mode
* World Gen: PCG (Procedural Content Generation) Framework + C++ Grid Actors
* Workflow: Git (GitHub Actions for CI/CD), Obsidian (Knowledge Management)

# 🏗 System Architecture

## 1. Modular Input Handling
To demonstrate clean separation of concerns, the input is split into three layers:
* Base Layer: Standard locomotion (WASD/Mouse).
* Arcane Layer: Pushed/Popped dynamically from the LocalPlayerSubsystem when the "Arcane Eyes" mode is active.
* Logic: Handled via UInputConfig Data Assets to avoid hardcoded string lookups in C++.

## 2. Reactive UI (MVVM)
The HUD does not use Tick or standard Event Construct binding.
* Viewmodel: A C++ UMVVMViewModelBase class tracks mana and vision states using the FIELD_NOTIFY macro.
* Common UI: Used for the "Victory" and "Pause" menus to manage input focus and UI layering automatically.

## 3. The "Spectral" Rendering Pipeline
* The Vision: A custom Post-Process pass using a Sobel-Edge detection algorithm.
* The Hidden Path: Actors utilize Gameplay Tags and Custom Depth Stencils. When the "Arcane" state is broadcast, these actors toggle their visibility and collision via a centralized StateComponent.

# 📅 Development Roadmap

See **[docs/Roadmap.md](docs/Roadmap.md)** for the full plan and per-day checklists.

**Sprint 1 — Core Vertical Slice**

| Day | Focus | Status |
|-----|-------|--------|
| Day 1 | Core Framework | ✅ Done |
| Day 2 | GAS + Spectral Shaders | ⬜ Not started |
| Day 3 | Grid & Interaction | ⬜ Not started |
| Day 4 | Data-Driven UI | ⬜ Not started |
| Day 5 | Portal & Polish | ⬜ Not started |

**Sprint 2 — Enhancement** *(optional)*

| Day | Focus | Status |
|-----|-------|--------|
| Day 6 | Animation (Motion Matching) | ⬜ Not started |

# 📚 Docs

| File | Contents |
|------|----------|
| [docs/Roadmap.md](docs/Roadmap.md) | 5-day sprint plan + per-day checklists |
| [docs/Architecture.md](docs/Architecture.md) | Class hierarchy, module deps, data-flow diagrams |
| [docs/SpectralVision.md](docs/SpectralVision.md) | Spectral Shift system: GameplayTags, StateComponent, Post-Process pipeline |
| [docs/UIArchitecture.md](docs/UIArchitecture.md) | MVVM ViewModel, Common UI stack, widget conventions |
| [AGENTS.md](AGENTS.md) | Guide for AI agents working in this codebase |

# 📝 Design Document: The "Arcane" Logic
Note to Recruiters: This project prioritizes Code Scalability. Instead of using simple booleans for the magic state, I implemented a Gameplay Tag Container. This allows the system to support multiple "vision modes" (e.g., Thermal, Chronal, Arcane) without modifying the base actor code.

# 🚀 How to Run
* Clone the repository.
* Right-click ThroughArcaneEyes.uproject -> Generate Visual Studio project files.
* Build the solution in Development Editor configuration.
* Play in Editor (PIE).