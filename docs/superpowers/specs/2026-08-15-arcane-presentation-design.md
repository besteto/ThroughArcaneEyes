# Arcane Presentation — Design

> Slots between **M2 — Mana Has Teeth** and **M3 — Arcane As A Sense**. Supersedes nothing; it assigns work the [restoration economy spec](2026-08-11-restoration-economy-design.md) §8 left unassigned, and implements §6.2 of the [connection loop spec](2026-08-10-connection-loop-design.md).

## 1. Summary

M1 and M2 built two mechanics that work and show almost nothing. A root grows along a spline with no indication of where the growth front is. A grove regenerates mana from a `UBoxComponent` that renders literally nothing — the recovery beat of M2's loop happens on an invisible patch of ground. Meanwhile the arcane look is spread across `M_SpectralEdge` and `M_GridCube_Arcane`, each holding its own copy of the colours, with nothing keeping them in agreement.

This pass gives the two live mechanics a presentation and puts the arcane palette in one place. It is deliberately limited to **things that already have behaviour underneath them**. Sapling glow waits for M3's saplings; restoration state changes and RVT vegetation blending wait for M4's healing. Building presentation for absent mechanics means guessing at what is being presented.

It also unblocks M2's deferred gate clip, which was postponed on 2026-08-15 because nothing in `WorldNull` was presentable.

---

## 2. Decisions

| # | Decision | Instead of |
|---|---|---|
| 1 | **A parameter contract, with Niagara systems authored in-editor** | C++ constructing emitters, or scripted asset generation |
| 2 | **`UTaeArcanePalette` data asset as the single source of truth, MPC as a derived copy** | The MPC being authoritative |
| 3 | **The palette ships first** | VFX first, palette later |
| 4 | **Geometry Script is out of scope** | Replacing the spline-mesh root with generated geometry |
| 5 | **`UGA_GrowRoot` tells the path when growth starts and stops** | `ATaeRootPath` polling for staleness |

### Why a parameter contract

C++ owns data and hooks; the `NS_` assets are authored in the editor against documented user parameters. This mirrors how the project already works — `ATaeRootPath::PathMesh` and `PathMaterial` are `EditAnywhere` and assigned in `BP_RootPath`, not hardcoded. The C++ side stays testable because parameter values are just floats and vectors, and the art iterates without recompiling.

Niagara emitter graphs are not practically scriptable, so there is no automation path here comparable to `tae_m2_assets.py`. Scaffolding empty `NS_` assets would produce shells, not starting points.

### Why the data asset is authoritative, not the MPC

Slate cannot read a Material Parameter Collection. M3's `STaeConnectionNetwork` overlay has to reproduce the arcane look in `OnPaint` (connection loop spec §6.3), which means reading the colours from something Slate can reach. A `UDataAsset` can be read by Slate, by C++, and copied into an MPC for materials; an MPC can only be read by materials. Making the MPC authoritative would strand the overlay.

### Why the palette ships first

Both Niagara systems read their colours from the MPC. Authoring them against hardcoded colours and retrofitting the MPC afterwards means re-authoring both systems. The dependency runs palette → MPC → VFX, so the work does too.

### Why Geometry Script is out

It would replace working M1 code — `ATaeRootPath::RebuildSplineMeshes` and the `USplineMeshComponent` per segment — with a new and unfamiliar system, for a visual improvement that the growth-front VFX delivers more cheaply. It remains a candidate for a later pass, where a failure degrades to the existing spline meshes rather than to nothing.

### Why the ability drives the growing flag

`UGA_GrowRoot` already knows exactly when a channel starts and ends. `ATaeRootPath` has no tick and should not gain one merely to notice that `AdvanceGrowth` stopped being called. An explicit `SetGrowing(bool)` from the ability is one call at each end of the channel and adds no per-frame cost.

---

## 3. Architecture

### 3.1 The palette

`UTaeArcanePalette : UDataAsset`, in `Public/Core/`. Holds the arcane linear colours in one place:

| Property | Consumed by |
|---|---|
| `SpectralEdge` | `M_SpectralEdge`, and M3's Slate overlay |
| `CubeTint` | `M_GridCube_Arcane` |
| `GroveBloom` | `NS_GroveBloom` |
| `GrowthFront` | `NS_GrowthFront` |

`UTaeArcaneSubsystem` is a `UWorldSubsystem` and has no details panel, so it cannot own an editable asset reference. `UTaeGameInstance` already solves this problem for audio — `Music_Forest` and `Music_Arcane` are `EditDefaultsOnly` there and handed out through getters. The palette follows the same pattern and is assigned in `BP_TaeGameInstance`.

`UTaeArcaneSubsystem::OnWorldBeginPlay` already locates the post-process volume; it also reads the palette off the game instance and writes each colour into `MPC_Arcane`. `M_SpectralEdge` and `M_GridCube_Arcane` are re-pointed at the collection, and both Niagara systems sample it. Agreement becomes structural rather than a matter of remembering to update two materials.

**Not in scope here:** driving MPC values from `ArcaneBlendAlpha` per frame. The palette is static colour data written once. The blend alpha already drives the post-process volume and the camera (`TaeArcaneSubsystem.cpp:90-104`) and does not need to route through the MPC to do so.

### 3.2 Grove bloom

`UTaeGroveComponent` gains `EditAnywhere TObjectPtr<UNiagaraSystem> BloomSystem`, assigned in `BP_Grove`. The component is spawned at `BeginPlay` with `UNiagaraFunctionLibrary::SpawnSystemAttached` rather than as a constructor default subobject, because a component creating child components in its own constructor is awkward and the grove's own construction already carries collision setup.

| User parameter | Type | Driven by | When |
|---|---|---|---|
| `Extent` | `Vector` | `GetScaledBoxExtent()` | once, `BeginPlay` |
| `RegenPerSecond` | `float` | `GetRegenPerSecond()` | once, `BeginPlay` |
| `IsOccupied` | `bool` | `ActiveRegen.Num() > 0` | in the existing overlap handlers |

The system sizes itself to the grove's real footprint, so a larger grove looks larger without a second authored value, and its density can scale with how generous the grove actually is. `IsOccupied` lets it react when the player is standing in it. No tick is added — the first two never change and the third rides the overlap callbacks that already exist.

`IsDataValid` gains a warning when `BloomSystem` is unassigned, alongside the existing `RegenCurve` error. A warning rather than an error: a grove with no VFX still functions, unlike a grove with no curve.

### 3.3 Growth front

`ATaeRootPath` gains `EditAnywhere TObjectPtr<UNiagaraSystem> GrowthFrontSystem`, assigned in `BP_RootPath`, plus a spawned component moved to the growing tip.

The tip position comes off the spline that already exists:

```
Distance = GrowthFrontDistance(GrowthAlpha, Spline->GetSplineLength())
Position = Spline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World)
```

`GrowthFrontDistance` is a pure static — `GrowthAlpha` clamped to 0..1, multiplied by length — so the arithmetic is testable without a spline, in the same spirit as `UTaeGroveComponent::RegenRateForArea` and `UTaeManaAttributeSet::EvaluateExhaustion`.

The position updates inside `AdvanceGrowth`, which runs only while a channel is active, so an idle path costs nothing.

| User parameter | Type | Driven by |
|---|---|---|
| *(component transform)* | — | moved to the tip position |
| `GrowthAlpha` | `float` | the path's `GrowthAlpha` |
| `IsGrowing` | `bool` | `SetGrowing`, called by `UGA_GrowRoot` |

`UGA_GrowRoot` calls `SetGrowing(true)` where it currently starts the growth timer and `SetGrowing(false)` in `EndAbility`, beside the existing drain removal. On `ETaeConnectionState::Restored` the front deactivates — a finished connection has no growing tip.

---

## 4. Testing

Thin, because most of this pass is data flowing into assets that only exist in the editor. What can be tested honestly:

| Test | Covers |
|---|---|
| `World.GrowthFront` | `GrowthFrontDistance` — clamping at both ends, midpoint, zero-length spline |
| `Core.ArcanePalette` | the palette writes every colour into the MPC, and a null palette leaves the collection untouched rather than zeroing it |

The palette test needs a real world. `FScopedTestWorld` currently lives inside `TaeManaDrainRateTest.cpp`; this is its second consumer, which is the right moment to promote it to `Private/Tests/TaeTestWorld.h` rather than copy it. That extraction is part of this work, not a separate cleanup.

The Niagara systems themselves are not tested. Their correctness is visual, and the gate clip is the check.

---

## 5. Parameter contract summary

The editor-side deliverable, in one place. Both systems read colour from `MPC_Arcane`.

**`NS_GroveBloom`** — attached to `UTaeGroveComponent`, spawned at `BeginPlay`.
`Extent` (Vector), `RegenPerSecond` (float), `IsOccupied` (bool).

**`NS_GrowthFront`** — attached to `ATaeRootPath`, moved to the tip.
`GrowthAlpha` (float), `IsGrowing` (bool).

Renaming a user parameter silently breaks the binding — `UNiagaraComponent::SetVariableFloat` and friends fail quietly on an unknown name. Names live in C++ as named constants so both sides have one spelling to match.

---

## 6. File structure

| File | Responsibility |
|---|---|
| `Public/Core/TaeArcanePalette.h` | **Create.** The data asset. Header only; no behaviour. |
| `Public/Core/TaeGameInstance.h` / `.cpp` | **Modify.** Hold the palette, expose a getter, following the music-asset pattern. |
| `Public/Core/TaeArcaneSubsystem.h` / `.cpp` | **Modify.** Read the palette in `OnWorldBeginPlay` and populate `MPC_Arcane`. |
| `Public/World/TaeGroveComponent.h` / `.cpp` | **Modify.** Bloom system, three parameters, `IsDataValid` warning. |
| `Public/World/TaeRootPath.h` / `.cpp` | **Modify.** Front system, `SetGrowing`, `GrowthFrontDistance`. |
| `Public/GAS/GA_GrowRoot.h` / `.cpp` | **Modify.** Call `SetGrowing` at both ends of the channel. |
| `Private/Tests/TaeTestWorld.h` | **Create.** `FScopedTestWorld`, extracted from `TaeManaDrainRateTest.cpp`. |
| `Private/Tests/TaeManaDrainRateTest.cpp` | **Modify.** Use the extracted header. |
| `Private/Tests/TaeGrowthFrontTest.cpp` | **Create.** |
| `Private/Tests/TaeArcanePaletteTest.cpp` | **Create.** |
| `ThroughArcaneEyes.Build.cs` | **Modify.** Add `Niagara` to `PublicDependencyModuleNames`. |
| `Tools/Python/tae_vfx_assets.py` | **Create.** Scripts `MPC_Arcane` and a `DA_ArcanePalette` instance. |

**This pass adds a module dependency.** `Niagara` is not currently referenced — `Build.cs:12-24` lists engine core, EnhancedInput, UMG/CommonUI/ModelViewViewModel, GAS, and GameplayCameras, with no FX module. `UNiagaraComponent` and `UNiagaraFunctionLibrary` both live in `Niagara`, so it goes in alongside the others. The M2 plan carried a global constraint reading "No new Build.cs modules; M2 adds no new dependencies" — that was scoped to M2 and does not bind here. No new *module* is created; one existing engine module is depended upon. The Material Parameter Collection needs nothing new, as `UMaterialParameterCollection` is in `Engine`.

**Editor-side, not scriptable:** `NS_GroveBloom`, `NS_GrowthFront`, re-pointing `M_SpectralEdge` and `M_GridCube_Arcane` at `MPC_Arcane`, and assigning the three new asset references in `BP_TaeGameInstance`, `BP_Grove`, and `BP_RootPath`.

---

## 7. Gate

One clip, and it is the M2 gate clip that was deferred — now with something on screen:

> Enter Arcane, the transition reads as one effect across post-process and camera. Channel at a break — the growth front throws motes as the root advances. Run dry, get ejected, walk to the grove — it is visibly a bloomed place, and it reacts as you step into it. Recover, return, finish the connection.

Recording it closes M2's outstanding artifact at the same time.

**Staging note carried from M2:** with the current `0.35` / `12` defaults a connection costs ~46 mana against a 100 bar, so running dry mid-channel does not happen by accident. Survey first to burn the bar down.

**Balance tuning is not part of this pass.** M2's step 5 was deferred until the visual pass existed, on the reasoning that how the economy reads depends on what is on screen. That condition is met *after* this work lands, not during it — so tuning is the next thing to pick up once the clip is recorded, and it is an editor-only pass over `BP_GA_GrowRoot`, `BP_GA_SpectralShift`, `BP_TaeCharacter`, and `Curve_GroveRegen` with no code behind it.

---

## 8. Out of scope

| Deferred to | Item |
|---|---|
| M3 | Sapling glow and persistent reveal marks — no saplings exist yet |
| M3 | The Slate overlay reproducing the arcane look (§6.3). This pass delivers the palette it will read |
| M4 | Restoration state changes via dynamic material instances — nothing heals yet |
| M4 | Runtime Virtual Textures for vegetation blending — same reason |
| M5 | Control Rig tree motion |
| Later | Geometry Script for the root mesh — see §2 |
| Later | A real character mesh, rock and ruin set dressing. Asset sourcing, not tech art |

**Already done, do not re-derive:** connection loop spec §6.4 describes a `FlashVignette` bug where the fade snapped to zero on a timer. It is fixed — `TaeArcaneSubsystem.cpp:72-77` interpolates `VignetteFlashAlpha` per tick.

---

## 9. Risks

| Risk | Mitigation |
|---|---|
| Niagara is a learning curve and could eat the pass | The palette work stands alone and ships first. If the VFX fight back, the grove degrades to a scrolling-material plane and the pass still delivers §6.2 |
| A renamed user parameter breaks a binding silently | Names are named constants in C++; §5 is the contract both sides match against |
| The grove VFX looks wrong at the authored `(700, 700)` extent | The system is fed the real extent rather than assuming one, so resizing the grove rescales it |
| Re-pointing two materials at the MPC could regress the existing look | Both materials are placeholder-grade already; the arcane look is post-process-dominant. Compare against a PIE capture before and after |
