# Project: Through Arcane Eyes (UE 5.8)
"Through Arcane Eyes" is a technical vertical slice developed in Unreal Engine 5.8. It serves as a portfolio demonstration of high-level C++ architecture, modern UI patterns (MVVM + Common UI), and advanced shader techniques.

You play as **Ant** — a humanoid living tree, roots for legs and branches for arms — stranded in a ruined factory world of floating cube islands. The land is broken, rusted, and overgrown. By activating **Arcane Vision** (the Spectral Shift), Ant perceives what could exist: hidden geometry, dormant connections, paths yet to grow. The goal is to restore the land — crossing and linking the separated islands by growing roots, vines, and trees between them.

# 🛠 Technology Stack
* Engine: Unreal Engine 5.8 (C++ 20)
* Input: Enhanced Input System (Modular Mapping Contexts)
* UI Architecture: Common UI (Input Routing) + UMG Viewmodel (Data Binding)
* Rendering: Post-Process Materials, Global Distance Fields, Stencil Buffers, Substrate Materials
* Gameplay: GAS (Gameplay Ability System) — Arcane toggle, root-growth channel, Mana attribute, Gameplay Tags
* Camera: Close over-the-shoulder third-person, blending to a pulled-back Arcane framing (`GameplayCameras`)
* Animation: Control Rig procedural tree motion — root drift, trunk bend, Arcane reach *(planned, M2)*
* World Gen: Hand-authored islands with PCG-generated detail and scatter *(planned, M4)*
* Testing: UE Automation Tests — pure game logic covered headlessly
* Workflow: Git (GitHub Actions for CI/CD), Obsidian (Knowledge Management)

> **Note on Motion Matching:** `PoseSearch` was evaluated and deliberately dropped. It is data-hungry
> and tuned for naturalistic humanoid locomotion — close to the opposite of what a walking tree needs.
> Control Rig produces distinctive, non-humanoid motion from far less source animation. The
> `UAF`/AnimNext stack was also assessed and rejected: every module ships under `Engine/Plugins/Experimental/` in 5.8.

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
* The Hidden Path: Actors use Gameplay Tags and a centralized `UTaeStateComponent`. When the `Arcane.Vision` tag is granted, dormant root connections reveal themselves as ghosts — the land Ant must restore.

## 4. The Connection Loop
The core verb of the game. Arcane Vision is both the diagnostic and the workshop.

* **See** — entering Arcane pulls the camera back and reveals the ghost root network between islands.
* **Grow** — standing at an `ATaeRootAnchor`, Ant channels (`UGA_GrowRoot`) and the root materialises segment by segment along a spline while mana drains.
* **Persist** — growth is permanent *and* partial. Release early and progress is kept; return later and resume. Mana pressure interrupts, it never punishes.
* **Cross** — at full growth the root becomes solid and walkable in normal mode.

State lives with the thing it describes: `ATaeRootPath` owns its own `GrowthAlpha` and `ETaeConnectionState`; `ATaeWorldManager` is a registry that counts and broadcasts. GAS owns the *verb*, plain actor state owns the *world* — a root path is a float and an enum, so giving it an ASC would buy nothing.

A single `ArcaneBlendAlpha` on `UTaeArcaneSubsystem` drives the post-process weight, the camera blend, and (later) the UI overlay from one interpolator, so the three can never disagree on timing.

## 5. The Restoration Economy
Two resources, two verbs — **Look** and **Plant**.

* **Mana** fuels both seeing and growing, and regenerates only near land you have already restored. It is the pressure that gives Forest mode a purpose.
* **Saplings** are not currency. Each is a specific living thing you found, revealed only through Arcane Vision, and it gates *where* you can grow rather than how much.

Completing a connection re-runs that region's PCG graph: vegetation spawns, rust recedes, and new saplings appear where there was nothing. **You never farm — you heal, and healing produces.** The world you restore is what pays for restoring more of it.

Full design in the **[restoration economy spec](docs/superpowers/specs/2026-08-11-restoration-economy-design.md)**.

# 📅 Development Roadmap

**Sprint 1 — Core Vertical Slice** ✅ Complete

| Day | Focus | Status |
|-----|-------|--------|
| Day 1 | Core Framework | ✅ Done |
| Day 2 | GAS + Spectral Shaders | ✅ Done |
| Day 3 | Grid + Third-Person Camera | ✅ Done |
| Day 4 | Data-Driven UI | ✅ Done |
| Day 5 | Portal & Polish | ✅ Done |

**Sprint 2 — Milestones**

Day-based planning was retired: the labels stopped describing reality. Milestones are now gated on a
recordable demo rather than a calendar. Full design in
**[the connection loop spec](docs/superpowers/specs/2026-08-10-connection-loop-design.md)**.

| Milestone | Focus | Status |
|-----------|-------|--------|
| M1 | Connection Loop Playable | ✅ Done |
| M2 | Mana Has Teeth — resource economy, GAS effects | ⬜ Not started |
| M3 | Arcane As A Sense — Slate overlay + sapling reveal | ⬜ Not started |
| M4 | The World Heals — PCG restoration | ⬜ Not started |
| M5 | Progression & Polish — win state, save, audio, Control Rig | ⬜ Not started |

**M1 gate:** reveal a broken root in Arcane Vision, channel it to half growth, release, return, finish
it, then walk across it in normal mode — verified in-editor.

# 📚 Docs

| File | Contents |
|------|----------|
| [docs/superpowers/specs/2026-08-11-restoration-economy-design.md](docs/superpowers/specs/2026-08-11-restoration-economy-design.md) | **Current design authority.** Resource economy, core loop, and the M2–M5 sequencing |
| [docs/superpowers/specs/2026-08-10-connection-loop-design.md](docs/superpowers/specs/2026-08-10-connection-loop-design.md) | Connection loop and growth mechanic — still authoritative for M1; its milestone ordering is superseded |
| [docs/superpowers/plans/2026-08-10-m1-connection-loop.md](docs/superpowers/plans/2026-08-10-m1-connection-loop.md) | M1 implementation plan, with corrections applied during execution |
| [docs/Roadmap.md](docs/Roadmap.md) | Sprint 1 record + per-day checklists *(Days 6–9 superseded by the spec)* |
| [docs/LevelGeneration.md](docs/LevelGeneration.md) | Level dressing design: `ATaeClutterScatter`, interactable spawners *(island approach superseded by the spec's hybrid PCG)* |
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