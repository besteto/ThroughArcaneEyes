# M1 Task 8 — Editor Handoff

> The C++ half of M1 is complete on `feature/m1-connection-loop` (17 commits, `2c84a53..15c569b`).
> Task 8 is editor work: input asset, Blueprints, camera rigs, level content, and the gate clip.
> Plan: [2026-08-10-m1-connection-loop.md](2026-08-10-m1-connection-loop.md) · Spec:
> [connection-loop-design](../specs/2026-08-10-connection-loop-design.md)

---

## Do this first

**Add `GameplayCameras` before anything else.** It is a `Build.cs` + `.uproject` edit requiring a full
rebuild (~50s). Doing it after building Blueprints means rebuilding mid-session.

```csharp
// ThroughArcaneEyes.Build.cs
PublicDependencyModuleNames.AddRange(new string[] { "GameplayCameras" });
```
```json
// ThroughArcaneEyes.uproject — Plugins array
{ "Name": "GameplayCameras", "Enabled": true }
```
Then rebuild and confirm `Result: Succeeded` with no "does not list plugin" warning.

---

## Naming gotcha

The input **asset** is `IA_GrowRoot` (matching the `IA_` asset convention). The C++ **property** it
plugs into is `GrowRootAction`, matching `MoveAction` / `SpectralShiftAction` / `PauseAction`. There is
no property called `IA_GrowRoot`.

---

## Assets to create

| Asset | Parent / notes |
|---|---|
| `Content/Character/Input/IA_GrowRoot` | `UInputAction`, Digital bool. Add to `IMC_Default` on **E** + Gamepad Face Button Right. Assign to `GrowRootAction` on `BP_TaePlayerController`. |
| `Content/GAS/BP_GA_GrowRoot` | Parent `UGA_GrowRoot`. Defaults are fine. Assign to `GrowRootAbility` on `BP_TaeCharacter`. |
| `Content/World/BP_RootPath` | Parent `ATaeRootPath`. Assign a placeholder cylinder to `PathMesh`, `M_GridCube_Arcane` to `PathMaterial`. |
| `Content/World/BP_RootAnchor` | Parent `ATaeRootAnchor`. |
| `Content/World/BP_WorldManager` | Parent `ATaeWorldManager`. |

**Level (`WorldNull.umap`):** keep the two existing cube islands as placeholder art (they are retired in
M4, not now). Place one `BP_RootPath` spanning the gap with 4–6 spline points; a `BP_RootAnchor` at each
end with `Path` set to that root path; one `BP_WorldManager` with the path in its `RootPaths` array.
Run **Build > Validate Data** — expect zero errors.

**Camera:** two GameplayCameras rigs (Forest = existing over-shoulder `TargetArmLength 80`,
`SocketOffset (0,50,20)`; Arcane = pulled back and raised), blended on
`UTaeArcaneSubsystem::GetArcaneBlendAlpha()` — now `BlueprintPure`, so it is readable in the editor.
Keep the existing `USpringArmComponent` until the rig is proven; it is the fallback.

---

## Verification checklist

Nine checks no agent could perform. Tasks 1–7 were verified by build + automation tests only; **no
visual behaviour has been confirmed.**

### From Task 3 — root path reveal
- [ ] Root path is invisible in Forest mode
- [ ] Toggling Arcane shows all segments as ghosts, none walkable
- [ ] `GetAll TaeRootPath GrowthAlpha` reports `0.0` at start

### From Task 6 — arcane blend
- [ ] Post-process **fades** in/out over ~0.35s rather than popping
- [ ] Vignette flash fades smoothly rather than snapping off

### From Task 7 — the channel
- [ ] Holding **E** at an anchor actually starts a channel *(confirms pawn-vs-anchor overlap survives the Blueprint subclasses — verified correct for native defaults only)*
- [ ] Mana drains while channelling, stops on release
- [ ] Releasing early keeps partial growth; returning resumes it
- [ ] Channelling from **either** anchor grows the same connection

---

## The M1 gate

1. Forest — root invisible, gap unwalkable
2. Toggle Arcane — post-process fades ~0.35s, camera pulls back, ghost root appears
3. Stand on near anchor, hold **E** — segments materialise one by one, mana drains
4. Release at ~half — growth stops, grown half stays solid
5. Toggle to Forest — grown half visible and walkable, ungrown half gone
6. Return to Arcane, hold **E** to completion — ability ends itself
7. Toggle to Forest, walk the full connection to the far island
8. Console `GetAll TaeWorldManager RestoredCount` → `1`

Record steps 2–7 as one take. **Mana budget checks out:** full growth is `1/0.35 = 2.86s` at 12 mana/s
≈ 34 of 100 starting mana, no regen — so half-grow → leave → return → finish fits in one pool.

---

## Known risks

**Riskiest untested assumption:** whether `USplineMeshComponent` deformed collision is actually walkable
after the runtime `NoCollision → QueryAndPhysics` flip (`TaeRootPath.cpp`). That is gate step 7. Static
analysis cannot establish it. If Ant falls through a fully grown root, this is why.

**Vignette flash may not read as intended.** `ApplyBlendAlpha` sets `BlendWeight = ArcaneBlendAlpha` and
writes `VignetteIntensity` into the same volume, so entering Arcane spikes the vignette while
`BlendWeight` is still ~0 (scaling the spike to nothing), and exiting drops `bEnabled` while the flash
is still decaying. The Task 6 check above may not reproduce. Belongs with M3's vignette work.

---

## Deferred minors

All triaged as safe to defer by the whole-branch review. Worth a cleanup pass whenever convenient.

| Where | What |
|---|---|
| `TaeConnectionTypes.h` | Comment references an "explicitly reset" path M1 does not deliver |
| `TaeRootAnchor.h` | `IsDataValid` under `public:`; `TaeGridCube`, `TaeRootPath`, `TaeWorldManager` all use `protected:` — this is the sole outlier |
| `TaeArcaneSubsystem.cpp` | Vignette decay re-implements `StepBlendAlpha` inline instead of calling it |
| `GA_GrowRoot.cpp` | `CanActivateAbility` does not gate on an already-`Restored` path; re-activating burns one tick (0.6 mana) before self-ending |
| `TaeCharacter.cpp` | Runtime `LogTae` null-guard where the house rule prefers `IsDataValid`; `SpectralShiftAbility` has neither |
| `TaeRootPath.cpp` | `SplineMeshSegments` is `Transient` + `OnConstruction`-only. Safe in PIE; a cooked build where construction does not re-run would leave segments visible and collidable in Forest mode. Pre-existing pattern, but M1 now depends on that array far more heavily. |
| Module-wide | `GetRestoredCount()`, `GetGrowthAlpha()`, `GetGrowthDirection()` currently have no callers — scaffolding for M3/M5. A conscious leave, not drift. |

---

## Defects found and fixed during execution

All three originated in the plan's own reference code, which was written without compiling. Recorded
because the pattern matters for M2 onward: **specs containing reference code need their semantics
reviewed, not just their transcription.**

1. `EConnectionState` collided with the engine's unscoped enum in `NetConnection.h` → `ETaeConnectionState`.
2. `StateFor` used an epsilon threshold, misclassifying `(0, 1e-4]` as `Broken` → strict zero.
3. **Backward anchors were an invisible mana sink.** `GrowthAlpha` is one scalar clamped to `[0,1]`, so a
   negative delta clamped back to 0 while mana had already been deducted — an indefinite drain that could
   never self-terminate, and on a partly-grown path it undid the other end's progress. Both ends now grow
   positively; `bGrowsForward` is retained as level data reserved for future visual use.

Two further gaps came out of the whole-branch review, both on seams no single-task review could see:
`GetArcaneBlendAlpha()` had no `UFUNCTION` (blocking the camera rig entirely), and `Arcane.Vision` gated
the channel's start but not its continuation — toggling Arcane mid-channel kept the root growing in
Forest mode.
