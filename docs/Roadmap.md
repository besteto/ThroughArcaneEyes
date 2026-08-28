# Development Roadmap — Through Arcane Eyes

> For coding conventions and patterns see [AGENTS.md](../AGENTS.md)

### Sprint 1 — Core Vertical Slice
*Playable portfolio demo: character, Arcane mechanic, grid world, UI, portal.*

| Day | Focus | Deliverable | Status |
|-----|-------|-------------|--------|
| Day 1 | Core Framework | C++ Character/Controller + Enhanced Input | ✅ Done |
| Day 2 | GAS + Spectral Shaders | GAS Foundation + Arcane Toggle + Post-Process | ✅ Done |
| Day 3 | Grid + Camera | `ATaeGridCube` + `ATaeGridManager` + over-shoulder camera | ✅ Done |
| Day 4 | Data-Driven UI | MVVM Viewmodel + Common UI HUD + Grid DataTable | ✅ Done |
| Day 5 | Portal & Polish | Render-to-Texture Portal + Win Condition UI | ✅ Done |

### Sprint 2 — Milestones

> **Day-based planning was retired on 2026-08-10.** The labels stopped describing reality — Day 6's Motion Matching was dropped outright, Day 7/8 shipped as one connection loop, and Day 9 slid behind the visual pass. Sprint 2 is now gated on a recordable demo rather than a calendar. The authority is [the restoration economy spec](superpowers/specs/2026-08-11-restoration-economy-design.md); the mechanic detail lives in [the connection loop spec](superpowers/specs/2026-08-10-connection-loop-design.md).

| Milestone | Focus | Status |
|-----------|-------|--------|
| M1 | Connection Loop Playable — `ATaeRootPath` growth, `UGA_GrowRoot`, `ArcaneBlendAlpha` | ✅ Done |
| M2 | Mana Has Teeth — mana attribute, periodic GEs, exhaustion, grove regen | ✅ Done |
| M3 | Arcane As A Sense — Slate connection overlay + sapling reveal | ⬜ Not started |
| M4 | The World Heals — PCG restoration on connection complete | ⬜ Not started |
| M5 | Progression & Polish — win state, save, audio, Control Rig | ⬜ Not started |

**M1 gate:** reveal a broken root in Arcane Vision, channel it to half growth, release, return, finish it, then walk across it in normal mode.

**M2 gate:** a ten-row PIE table covering drain, regen, exhaustion entry and recovery, and the grove — verified 2026-08-15. Its demo clip is deferred to the arcane presentation pass, because nothing in `WorldNull` was presentable enough to record.

The day-by-day records below are kept as a build log of Sprint 1. **Days 6–9 are superseded** — see [what happened to them](#what-happened-to-days-69).

---

## Day 1 — Core Framework ✅

**Goal:** Playable character in-editor with all input wired end-to-end.

**All complete.**

- C++ framework classes — `ATaeCharacter`, `ATaePlayerController`, `ATaeGameMode` (`AGameMode`), `ATaeGameState`, `UTaeGameInstance`, `ATaeHud`
- Enhanced Input — Move / Look / Jump / `DoSpectralShift` stub bound in controller; WASD Swizzle/Negate modifiers correct
- `LogTae` project log category; null-guard warnings on all BP-assigned properties → migrated to `IsDataValid` on `ATaePlayerController`
- Blueprint parenting — `BP_TaeGameMode`, `BP_TaeCharacter`, `BP_TaePlayerController`, `BP_TaeGameState`, `BP_TaeGameInstance`, `BP_TaeHud` created with correct parents; class refs set in `BP_TaeGameMode` Class Defaults
- Input assets — `IMC_Default`, `IA_Move`, `IA_Look`, `IA_Jump`, `IA_SpectralShift`
- Config — `GlobalDefaultGameMode`, Enhanced Input component class, `GameInstanceClass` set
- `UTaeMainMenuWidget` (`UCommonActivatableWidget`) + `WBP_MainMenu` placeholder — Start (DeactivateWidget) / Exit (QuitGame); shown via `ATaeHud::BeginPlay`; `CommonUI` added to `Build.cs`

---

## Day 2 — GAS + Spectral Shaders

**Goal:** GAS drives the Arcane toggle; pressing a key activates `GA_SpectralShift`, grants `Arcane.Vision` tag, and triggers the post-process pass.

**Before starting — loose ends from Day 1:** ✅ All clear

**All complete.**

### C++ / GAS
- GAS deps, ASC + `UTaeManaAttributeSet` + `UGA_SpectralShift` + `UTaeStateComponent`
- `UTaeGameplayAbility` abstract base — `AbilityDuration` timer (`0` = player-cancel only); all future abilities inherit from this
- `UGA_SpectralShift` → `UTaeGameplayAbility`; toggle via `TAG_Arcane_Vision`; `IMC_Arcane` push/pop in Activate/End
- Native tag `TAG_Arcane_Vision` declared in `TaeGASTypes.h`, defined in `TaeGASTypes.cpp` — no `FName` strings in calling code
- `ATaeCharacter` exposes `SpectralShiftHandle`; controller uses `TryActivateAbility` / `CancelAbilityHandle` — no class-based lookup
- `SpectralShiftAbility` (`TSubclassOf`) on `ATaeCharacter` — assign `BP_GA_SpectralShift` in `BP_TaeCharacter` Class Defaults

### Materials / Rendering
- `M_SpectralEdge` — animated plasma overlay; DDX/DDY depth edge mask; `Floor(Time)` stepped animation
- Infinite Extent post-process volume — chromatic aberration; disabled by default. Placed directly in `WorldNull.umap` as a native `APostProcessVolume`; `UTaeArcaneSubsystem` finds it at `OnWorldBeginPlay`, so there is no Blueprint wrapper

---

## Day 3 — Grid + Camera ✅

**Goal:** Procedural cube grid loads and reacts to Arcane Vision. Close over-shoulder third-person camera. Devlog gif ready.

**Complete** — mesh collision deferred; placeholder meshes are complex shapes, add box/convex collision per mesh when final assets are ready.

### C++
- `ATaeGridCube` — Actor with `UStaticMeshComponent` + `UTaeStateComponent`; responds to `OnStateChanged`; toggles `ECollisionEnabled::NoCollision` when hidden, restores on Arcane off
- `ATaeGridManager` — spawns an N×M×K grid of `ATaeGridCube` from configurable `UPROPERTY` dimensions; no DataTable yet
- `ATaeCharacter` — `USpringArmComponent` + `UCameraComponent` close over-shoulder setup (replaces first-person camera)

### Materials
> Two separate materials swapped by `ATaeGridCube` via `UStaticMeshComponent::SetMaterial()` on `OnStateChanged`.
- `M_GridCube_Forest` — default world state; Surface opaque, organic greens and rock tones
- `M_GridCube_Arcane` — Arcane vision state; Surface translucent with emissive glow

### Editor
- `BP_TaeGridCube` → parent `ATaeGridCube`; test cube mesh + materials assigned (rework planned with detailed materials)
- `BP_TaeGridManager` — placed in level
- `BP_TaeCharacter` — tree skeletal mesh + animation assigned

---

## Day 4 — Data-Driven UI ✅

**Goal:** HUD reacts to mana/vision state with zero Tick usage. Grid layout driven by DataTable.

**All complete.**

### C++ — MVVM
- `ModelViewViewModel` added to `Build.cs`; `UTaeStateComponent` migrated to `TAG_Arcane_Vision` native tag
- `UTaeHudViewModel` — `UMVVMViewModelBase`; `Mana` (float) + `bArcaneActive` (bool) with `FieldNotify`; setters use `UE_MVVM_SET_PROPERTY_VALUE`
- `UTaeGameInstance` — creates and owns `UTaeHudViewModel` in `Init()`; `GetHudViewModel()` exposed as `BlueprintCallable`
- `ATaePlayerController::SetPawn` — wires ASC tag + Mana attribute delegates → ViewModel via `AddWeakLambda`; pushes initial values on possession
- `ATaeHud` — `HudWidgetClass` (`TSubclassOf<UUserWidget>`); creates and adds `WBP_HUD` to player screen in `BeginPlay`

### Editor / UMG
- `WBP_HUD` — View Bindings: `ManaText` → `TextBlock.Text`, `ArcaneVisibility` → widget visibility; Event Construct sets ViewModel from GameInstance
- `BP_TaeHud` — `WBP_HUD` assigned to `HudWidgetClass`
- `WBP_PauseMenu` — `UTaeActivatableWidget`; Escape via `IA_Pause` → `ATaeHud::TogglePauseMenu`
- `WBP_WinScreen` — `UTaeActivatableWidget`; "To Main Screen" → `ATaeHud::ShowMainMenu`
- `UTaeActivatableWidget` — base class; auto collapse/visible on deactivate/activate; `WBP_MainMenu`, `WBP_PauseMenu`, `WBP_WinScreen` all inherit


---

## Day 5 — Portal & Polish ✅

**Goal:** Functional end-portal with render-to-texture view; win condition triggers Victory screen.

**Complete** — render-to-texture portal, `M_Portal`, and the `M_GridCube_Forest` texture upgrade deferred (not needed for win condition).

### C++
- `ATaePortal` — Actor with `USphereComponent` trigger; overlap calls `ATaeHud::ShowVictoryScreen()`
- `UTaeArcaneSubsystem` (`UWorldSubsystem`) — auto-finds `APostProcessVolume` in `OnWorldBeginPlay`; `SetArcaneActive` toggles volume + crossfades music; `FlashVignette` spikes vignette intensity then fades
- `UTaeGameInstance` — `Music_Forest`, `Music_Arcane` (EditDefaultsOnly), `MusicCrossfadeDuration`; `GA_SpectralShift` calls subsystem instead of holding direct PP reference

### Deferred
> Carried forward to [Still owed from Sprint 1](#still-owed-from-sprint-1).

### Audio
> Asset import and wiring resequenced to M5 — see [what happened to Days 6–9](#what-happened-to-days-69). Music crossfade C++ logic is complete.
- Music crossfade logic — `UAudioComponent` pair in `UTaeArcaneSubsystem`; spawned in `OnWorldBeginPlay`; crossfaded via `SetArcaneActive`

### Substrate Upgrade
- `M_GridCube_Forest` → Substrate opaque slab
- `M_GridCube_Arcane` → Substrate translucent slab with emissive layer
- `M_SpectralEdge` — verified and converted to Substrate

### Polish
- Screen-space vignette flash when Spectral Shift activates — `UTaeArcaneSubsystem::FlashVignette()`
- `DefaultGame.ini` `ProjectVersion` bump to `0.1.0`

---

## What happened to Days 6–9

Each of the four planned days was resolved — dropped, delivered under a different name, or resequenced. None is still pending in its original form.

| Original day | Outcome | Where it went |
|---|---|---|
| **Day 6** — Animation (Motion Matching) | ❌ **Dropped** | `PoseSearch` is data-hungry and tuned for naturalistic humanoid locomotion, which is close to the opposite of what a walking tree needs. Replaced by Control Rig procedural tree motion, resequenced to **M5**. The `UAF`/AnimNext stack was assessed at the same time and rejected — every module ships under `Engine/Plugins/Experimental/` in 5.8. |
| **Day 7** — Level Dressing & Clutter | 🔀 **Partly delivered, partly resequenced** | `ATaeClutterScatter` and `ATaeInteractableSpawnPoint` exist in `World/`. Hand-placed set dressing is blocked on art — there are still no rock or ruin meshes in `Content/`. The wider approach is superseded by the spec's hybrid PCG islands in **M4**; design notes remain in [LevelGeneration.md](LevelGeneration.md), sourcing candidates in [VisualAssets.md](VisualAssets.md). |
| **Day 8** — Interactables & Connections | ✅ **Delivered as M1** | The root-connection mechanic shipped whole: `ATaeRootPath`, `ATaeRootAnchor`, `ATaeWorldManager`, `ETaeConnectionState`, and `UGA_GrowRoot`. It grew past a day's worth of scope and became the milestone the rest of Sprint 2 hangs off. `ATaeInteractableSpawner` exists but has no `BP_Chest` or `BP_Pickup_Mana` behind it yet. |
| **Day 9** — Audio & SFX | ⏳ **Resequenced to M5** | The C++ side is done — `UTaeArcaneSubsystem` spawns the two music components and crossfades them on `SetArcaneActive`. Only asset import and assignment remain. There are still no sound assets in `Content/`. Candidate source files per cue are catalogued in [AudioAssets.md](AudioAssets.md). |

### Still owed from Sprint 1

Small deferrals that never blocked anything and are still open:

- [ ] `ATaePortal` render-to-texture — `USceneCaptureComponent2D` + `UTextureRenderTarget2D`, and `M_Portal` to sample it
- [ ] `M_GridCube_Forest` texture upgrade — replace Voronoi with tiling rust and moss textures plus normal maps
- [ ] Grid mesh collision — placeholder meshes are complex shapes; add box or convex collision once final assets land
