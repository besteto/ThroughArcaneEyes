# Connection Loop — Design Spec

> Reworks the Sprint 2 plan around four pillars: an animated Ant, generated islands, seeing broken
> connections, and restoring them. Supersedes Days 6–9 in [Roadmap.md](../../Roadmap.md) and the
> hand-placed-only proposal in [LevelGeneration.md](../../LevelGeneration.md).
>
> Engine: UE 5.8. Plugin maturity verified against `D:\EpicGames\UE_5.8\Engine\Plugins` on 2026-08-10.

---

## 1. Summary

Ant restores a ruined world by regrowing the root connections between floating islands. Arcane Vision
is both the diagnostic and the workshop: entering it pulls the camera back, reveals the ghost root
network with its breaks, and is the only mode in which roots can be grown. Mana drains while it is
active, so the player cycles between surveying-and-repairing in Arcane and traversing-and-gathering in
Forest.

The novel part of this project is the connection loop. Everything else is in service of it.

---

## 2. Decisions

| # | Decision | Rejected alternatives |
|---|---|---|
| 1 | **Hybrid PCG islands** — artist authors silhouette, hero landmarks, and connection anchors; PCG generates surface detail and scatter | Full end-to-end PCG generation; hand-placed-only |
| 2 | **Channel-to-grow** — hold at a break to grow the root along its spline while mana drains | Traverse-under-pressure; gather-and-spend seeds |
| 3 | **Control Rig procedural tree motion** — blendspace base, Control Rig for sway/bend/reach | PoseSearch Motion Matching; UAF/AnimNext |
| 4 | **Arcane pulls the camera back; both seeing and growing happen there** | Top-down survey / over-shoulder act split; no camera change |
| 5 | **Milestones gated on a playable demo**, no calendar estimates | Re-cut Day N framing |
| 6 | **Slate for the connection overlay** — one paint routine, both camera modes | UMG widgets per connection |

### Why not UAF / Mover

Every `UAF/*` module and both `Mover` plugins ship under `Engine/Plugins/Experimental/` in 5.8.
`PCG` (core), `PoseSearch`, `GameplayCameras`, `GeometryScripting`, `Chooser`, and `MotionWarping` are
production. The animation and movement pillars have to work, so they stay on production plugins.
`PCGBiomeCore` is experimental and is **not** a dependency — plain `PCG` is.

### Why not Motion Matching

Motion Matching is data-hungry and tuned for naturalistic humanoid locomotion. The project owns a
placeholder Stickman rig with roughly a dozen clips, and `story_concept` explicitly calls for
"root drift + sway idle > standard walk cycle". Control Rig produces tree-specific motion from far less
source data and doubles as the rig for root-growth deformation.

---

## 3. Core Loop

```
FOREST  (default, free)
  over-shoulder camera, close
  traverse already-restored roots, explore islands, gather
        |
        |  toggle Arcane  (mana begins draining)
        v
ARCANE  (expensive, timed)
  camera blends back and up
  ghost root network visible, breaks marked
  hold at a break -> root grows along spline, mana drains faster
        |
        |  mana empties -> forced back to Forest
        v
  grown segments persist in BOTH modes
```

Growth is **permanent and partial**. Releasing the channel early leaves the root grown to whatever
`GrowthAlpha` was reached; the player can return and continue. This is what makes mana pressure
interesting rather than punishing — progress is never lost, only interrupted.

A connection becomes traversable in Forest mode only at `GrowthAlpha == 1`.

---

## 4. Architecture

State lives with the thing it describes; the manager is a registry, not a god object.

```
ATaeRootPath                      one hidden root connection (exists today)
  USplineComponent                  authored by designer
  UTaeStateComponent                already listens for Arcane.Vision
  EConnectionState                  Broken | Growing | Restored
  float GrowthAlpha                 0..1, persists across mode toggles
  -> drives USplineMeshComponent visibility + collision

ATaeRootAnchor                    NEW. marker at each end of a path; the spot the player channels from

ATaeWorldManager                  registry (currently an empty shell — this gives it a job)
  TArray<ATaeRootPath*>             registered at BeginPlay
  OnNetworkChanged delegate         broadcasts when any path's state changes
  int32 RestoredCount / RequiredCount

ATaeGameState                     win condition: RestoredCount >= RequiredCount

UGA_GrowRoot : UTaeGameplayAbility   NEW. the verb
  activates only while Arcane.Vision is active and Ant overlaps an ATaeRootAnchor
  drains mana per second, advances the target path's GrowthAlpha
  ends on release, mana exhaustion, or GrowthAlpha == 1

UTaeHudViewModel                  observes ATaeWorldManager -> feeds the Slate overlay
```

**GAS owns the verb; plain actor state owns the world.** The channel is an ability because mana cost,
duration, and cancellation already live in `UTaeGameplayAbility`. A root path is a bool and a float —
giving it an ASC would be heavy for no benefit.

Naming follows the existing split: the *action* is the shift/grow name (`GA_GrowRoot`, `IA_GrowRoot`,
`DoGrowRoot`), the *state* is the world name (`Arcane.Vision`). New native tags go in `TaeGASTypes.h`
beside `TAG_Arcane_Vision` — no `FName` strings.

> Naming note: `GrowRoot` is a placeholder. Per the project's naming rule, replace it with whatever the
> mechanic is actually called in the README once that name is settled.

---

## 5. Slate Connection Overlay

### Rationale

Not a minimap. `ATaeRootPath` already owns an `FSplineCurve`; the overlay projects **those same
control points** to screen space and paints them, with `GrowthAlpha` driving the colour array. The 3D
root and the 2D arc are one dataset with two renderers, so they cannot disagree.

Three independent justifications:

1. **Diegetic** — Arcane Vision is Ant perceiving the world's root system. The overlay *is* the vision,
   not chrome describing it. The network graph appears only in the pulled-back camera mode.
2. **Technically unavoidable** — UMG has no line or spline primitive. M curved edges, each partially
   filled, would mean M materials or mesh hacks.
3. **Forced by decision 1** — island and connection counts are *generated*, so the correct number of
   widgets cannot be authored at design time. A paint-based widget is indifferent to whether there are
   3 connections or 30. This argument survives even if the other two are dropped.

### Primitives

Verified in `Engine/Source/Runtime/SlateCore/Public/Rendering/DrawElementTypes.h`:

```cpp
MakeLines(ElementList, Layer, Geom, Points, PointColors, Effects, Tint, bAntialias, Thickness); // :262
MakeCubicBezierSpline(ElementList, Layer, Geom, P0, P1, P2, P3, Thickness, Effects, Tint);      // :210
MakeBox(...)  // :102     MakeText(...)  // :134
```

The `MakeLines` per-point-colour overload is the key primitive: one curved edge, coloured
restored-vs-broken along its length with the gradient sitting exactly at the growth frontier, in a
single draw call.

### Structure

```
STaeConnectionNetwork : SLeafWidget      pure paint, no children
  OnPaint -> MakeLines(pts, colors)        edges filled to GrowthAlpha
          -> MakeBox / MakeText            island nodes, break markers

UTaeConnectionNetworkWidget : UWidget    UMG wrapper
  RebuildWidget() -> SNew(STaeConnectionNetwork)
  reads UTaeHudViewModel; placed in WBP_HUD beside existing bindings
```

**One widget, two draw states.** The widget class is always present; what it paints depends on mode.
In Forest it draws nothing unless a channel is active, in which case it paints only the growth radial.
In Arcane it expands the same paint routine to the full network graph. One class, one `OnPaint`, no
second UI system — but the network graph itself is Arcane-only, per justification 1.

### Hooks

- `Build.cs:24` already has the `Slate, SlateCore` dependency line commented out — uncomment it.
- `AGENTS.md`'s naming table needs one row: Slate widget → `S` + `Tae` → `STaeConnectionNetwork`.

### Cost

`SLeafWidget` + `UWidget` wrappers are more C++ than a UMG binding. Screen-projecting world splines
needs a projection cached once per frame, not a naive per-point `ProjectWorldToScreen`.

---

## 6. Arcane Visual Coherence

**Slate composites after post-processing.** The overlay does not inherit the Arcane look automatically;
it must reproduce it. This is why matching is a design task rather than a styling pass.

### 6.1 One transition alpha

`UTaeArcaneSubsystem::SetArcaneActive` is currently a hard binary switch
(`SpectralVolume->bEnabled = bActive`, `TaeArcaneSubsystem.cpp:44`). With a camera blend and an overlay
fade added, three systems would each run on their own timing and visibly disagree.

Introduce a single `ArcaneBlendAlpha` (0..1) owned by `UTaeArcaneSubsystem`, interpolated over one
`ArcaneTransitionDuration`. All three consumers read it:

| Consumer | Driven by |
|---|---|
| Post-process | `APostProcessVolume::BlendWeight` — replaces the binary `bEnabled` |
| Camera | GameplayCameras blend weight between the Forest and Arcane rigs |
| Slate overlay | Opacity and colour lerp in `OnPaint` |

### 6.2 Shared palette

`UTaeArcanePalette` (a `UDataAsset`) holds the Arcane linear colours in one place. The Slate widget
style reads it directly; a Material Parameter Collection is populated from it at `BeginPlay` so
`M_SpectralEdge` and `M_GridCube_Arcane` draw from the same values. Slate cannot read an MPC, which is
why the data asset — not the MPC — is the source of truth.

### 6.3 Reproducing the effects in Slate

- **Chromatic aberration** — draw each edge three times: the base colour, plus red and blue passes at a
  small screen-space offset scaled by `ArcaneBlendAlpha`. Same `MakeLines` primitive, so the overlay
  carries the identical aberration as the post-process volume.
- **Vignette** — the overlay dims toward screen edges in its own paint, since the post-process vignette
  cannot reach it.
- **Stepped animation** — `M_SpectralEdge` uses a `Floor(Time)` stepped pulse; the overlay quantises its
  own pulse to the same interval so both read as one effect.

### 6.4 Bug to fix en route

`FlashVignette`'s header comment says "Duration is total fade-out time", but `ClearVignetteFlash` snaps
`VignetteIntensity` straight to `0.f` (`TaeArcaneSubsystem.cpp:76`) after the timer. Comment and code
disagree. Make it actually interpolate, driven by the same alpha machinery as 6.1.

---

## 7. Hybrid PCG Islands

**Artist authors:** island silhouette, 2–3 hero ruins, `ATaeRootAnchor` positions, portal placement.
**PCG generates:** surface rocks and rubble, vine and wire clutter, ground scatter, edge crumble detail.

Puzzle layout stays hand-controlled — connection topology is authored, never generated — while visual
density is procedural. Anchors are authored precisely because the loop depends on them being
reachable and legible.

`ATaeClutterScatter` is replaced by PCG and deleted. Its `FRandomStream` seeding idea survives as the
PCG graph's seed parameter.

Dependency: `PCG` only. Not `PCGBiomeCore` (experimental).

---

## 8. Ant Animation

Base: a small locomotion blendspace (idle/walk) on the existing rig.
Layer: Control Rig for the tree-specific motion —

- branch sway driven by noise
- trunk bend toward movement direction
- root-toe drift on footfalls
- limbs extend and reach when `Arcane.Vision` is active

The Arcane reach pose blends on the same `ArcaneBlendAlpha` from §6.1, so Ant's posture transitions in
lockstep with the camera, post-process, and overlay.

The Control Rig also drives root-growth deformation in §4, so it is built once and used twice.

---

## 9. Camera

`GameplayCameras` (production) with two rigs and a blend driven by `ArcaneBlendAlpha`:

- **Forest** — the existing over-shoulder setup: `TargetArmLength = 80`, `SocketOffset = (0, 50, 20)`
- **Arcane** — pulled back and raised, framing the local island cluster and its connections

The existing `USpringArmComponent` + `UCameraComponent` on `ATaeCharacter` are retained until the Arcane
rig is proven in M1, then removed. They are not deleted up front — the spring arm is the fallback if
GameplayCameras does not work out.

---

## 10. Retirement

| Item | Action |
|---|---|
| `ATaeGridCube` | Delete. Its reveal pattern already lives on in `ATaeRootPath`. |
| `ATaeGridManager` | Delete. Marked retired in `LevelGeneration.md` but never removed. |
| `ATaeClutterScatter` | Delete. Replaced by the PCG graph (§7). |
| `BP_TaeGridCube`, `BP_TaeGridManager` | Delete. |
| Cube islands in `WorldNull.umap` | Rebuild as authored islands per §7. |
| `UCharacterInfo` (`Data/CharacterInfo.h`) | Delete. Empty stub, violates the `Tae` prefix rule. |
| `M_GridCube_Forest` / `M_GridCube_Arcane` | Keep — retarget to the authored island meshes. |

Doc drift to correct in the same pass: `AGENTS.md` calls the project "first-person" (it is
over-shoulder third-person), omits `ModelViewViewModel` from its Build.cs list, cites `UCharacterInfo`
as the example of a convention it breaks, and refers to a `BP_Hero` that does not exist — the asset is
`BP_TaeCharacter`. `Roadmap.md` Day 1 still lists pre-rename `IA_MoveInputAction` names.

---

## 11. Milestones

Each ends in something recordable. No calendar estimates. Ordered so the core loop is playable before
any polish, which also de-risks the PCG and Control Rig learning curves.

> This spec describes a program, not a single implementation plan. **M1 is the first plan unit**; each
> later milestone gets its own plan when its predecessor's gate is met. Milestones after M1 are
> deliberately less detailed — they will be re-specified against what M1 actually teaches.

### M1 — Connection Loop Playable

The whole point of the project, proven on throwaway art.

- `ATaeRootPath` gains `EConnectionState` + `GrowthAlpha`
- `ATaeRootAnchor` marker actor
- `ATaeWorldManager` becomes a real registry with `OnNetworkChanged`
- `UGA_GrowRoot` + `IA_GrowRoot` + `DoGrowRoot`, mana drain, partial persistence
- `ArcaneBlendAlpha` in `UTaeArcaneSubsystem`; post-process moves to `BlendWeight`
- GameplayCameras Forest/Arcane rigs blended on that alpha
- **One hand-built island pair, placeholder meshes**

> Gate: clip showing reveal → grow → release early → return → finish → cross in Forest mode.

### M2 — Ant Moves Like A Tree

- Locomotion blendspace on the existing rig
- Control Rig: sway, trunk bend, root drift, Arcane reach on `ArcaneBlendAlpha`

> Gate: clip of Ant walking and toggling Arcane, posture shifting with the camera.

### M3 — The Network Is Legible

- `STaeConnectionNetwork` + `UTaeConnectionNetworkWidget`, uncomment `Build.cs:24`
- `UTaeArcanePalette` + MPC population
- Chromatic aberration, vignette, and stepped pulse reproduced in paint (§6.3)
- `FlashVignette` interpolation fix

> Gate: clip of the overlay fading in with the camera, edges filling as a root grows.

### M4 — Islands Become Generated

- PCG graph: surface scatter, clutter, edge detail from authored anchors and silhouette
- Delete `ATaeClutterScatter` and the grid cube classes
- Rebuild `WorldNull.umap` with 3–4 authored islands

> Gate: clip of an island rerolling from a new seed with anchors and layout intact.

### M5 — World, Progression, Polish

- `ATaeGameState` win condition on `RestoredCount >= RequiredCount`
- Interactable spawner wired to authored spawn points
- Audio pass (the existing Day 9 list — music crossfade C++ is already done)
- Doc drift corrections from §10

> Gate: full playthrough clip — restore every connection, portal opens, victory screen.

---

## 12. Risks

| Risk | Mitigation |
|---|---|
| Control Rig tree motion is a new skill and may not look good quickly | M1 does not depend on it; M2 can ship with the blendspace alone if the rig disappoints |
| PCG has a real learning curve | Deferred to M4, after the loop is proven. Authored anchors mean a PCG failure degrades to hand-placed dressing, not a broken game |
| Screen-projecting splines every frame could cost | Cache the projection once per frame; the network graph only paints in Arcane mode |
| GameplayCameras replaces a working spring-arm setup | Keep the spring arm until the Arcane rig is proven, then remove |
| Mana drain rates make the loop tedious or trivial | All exposed as `EditAnywhere` on the ability; tune in M1 before building content on top |

## 13. Open Questions

- The mechanic name. `GrowRoot` is a placeholder; the project's naming rule wants the README's own
  vocabulary, which does not yet have a word for this verb.
- Whether restored connections should persist across level reload (save game) or only within a session.
  M1 assumes session-only; nothing in the architecture blocks adding persistence later.
- Whether a Slate **editor** panel for authoring root paths is worth a separate editor module. It would
  be genuine Slate work but contradicts `AGENTS.md`'s "no new Build.cs modules" rule. Out of scope here.
