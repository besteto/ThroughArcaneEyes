# Project: Through Arcane Eyes (UE 5.7)
"Through Arcane Eyes" is a technical vertical slice developed in Unreal Engine 5.7. It serves as a portfolio demonstration of high-level C++ architecture, modern UI patterns (MVVM + Common UI), and advanced shader techniques.

You play as **Ant** — a humanoid living tree, roots for legs and branches for arms — stranded in a ruined factory world of floating cube islands. The land is broken, rusted, and overgrown. By activating **Arcane Vision** (the Spectral Shift), Ant perceives what could exist: hidden geometry, dormant connections, paths yet to grow. The goal is to restore the land — crossing and linking the separated islands by growing roots, vines, and trees between them.

# 🛠 Technology Stack
* Engine: Unreal Engine 5.7 (C++ 20)
* Input: Enhanced Input System (Modular Mapping Contexts)
* UI Architecture: Common UI (Input Routing) + UMG Viewmodel (Data Binding)
* Rendering: Post-Process Materials, Global Distance Fields, Stencil Buffers, Substrate Materials
* Gameplay: GAS (Gameplay Ability System) — Arcane toggle, Mana attribute, Gameplay Tags
* Camera: Close over-the-shoulder third-person (`USpringArmComponent`)
* Animation: Motion Matching (PoseSearch) — tree locomotion (root gait) + Arcane root-reach/extend poses
* World Gen: C++ procedural grid actors (`ATaeGridCube`, `ATaeGridManager`)
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
* The Vision: A custom Post-Process pass — animated plasma overlay with DDX/DDY depth edge detection and chromatic aberration — activates when Arcane Vision is toggled.
* The Hidden Path: Actors use Gameplay Tags and a centralized `UTaeStateComponent`. When the `Arcane.Vision` tag is granted, hidden cube islands toggle their visibility and collision — revealing the dormant land Ant must restore.

# 📅 Development Roadmap

See **[docs/Roadmap.md](docs/Roadmap.md)** for the full plan and per-day checklists.

**Sprint 1 — Core Vertical Slice**

| Day | Focus | Status |
|-----|-------|--------|
| Day 1 | Core Framework | ✅ Done |
| Day 2 | GAS + Spectral Shaders | ✅ Done |
| Day 3 | Grid + Third-Person Camera | ✅ Done |
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
| [CONTRIBUTING.md](CONTRIBUTING.md) | Commit format, code style, naming conventions |
| [AGENTS.md](AGENTS.md) | Guide for AI agents working in this codebase |

# 📝 Design Note: The "Arcane" Logic
Arcane Vision is the mechanical expression of Ant's nature — a living thing that senses the latent life in a dead world. In normal mode the factory islands look rusted and disconnected; through Arcane Eyes, the hidden geometry of what *could grow* becomes visible and physical.

**Note to Recruiters:** The vision state is driven by a Gameplay Tag Container (`Arcane.Vision`), not a boolean. This means the system naturally supports additional modes (Thermal, Chronal, etc.) without touching base actor code — each mode is just a new tag + a new `GA_` ability that grants it.

# 🚀 How to Run
* Clone the repository.
* Right-click ThroughArcaneEyes.uproject -> Generate Visual Studio project files.
* Build the solution in Development Editor configuration.
* Play in Editor (PIE).

# 📄 License
Copyright © 2026 Helen Allien Poe. **Source Available** — see [LICENSE](LICENSE).

The source code is public for portfolio and educational viewing only.
Redistribution, commercial use, or inclusion in other projects is not permitted.