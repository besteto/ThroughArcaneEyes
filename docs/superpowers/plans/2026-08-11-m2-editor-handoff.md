# M2 — Editor Handoff

> The C++ half of M2 is complete on `feature/m2-mana-economy` (Tasks 1–9, `43ce948..0db11df`).
> Every asset except the level placement and the HUD widget was scripted by
> [`Tools/Python/tae_m2_assets.py`](../../../Tools/Python/tae_m2_assets.py).
> Plan: [2026-08-11-m2-mana-economy.md](2026-08-11-m2-mana-economy.md) · Spec:
> [m2-mana-economy-design](../specs/2026-08-11-m2-mana-economy-design.md)

---

## Already done — do not redo

| Asset | State |
|---|---|
| `Content/GAS/Cues/GC_Mana_Drain` / `GC_Mana_Regen` / `GC_Arcane_Exhausted` | Created, parented to the `UTaeCueNotify_*` classes |
| `Content/World/Curve_GroveRegen` | Created with keys `50 → 4`, `200 → 10` |
| `Content/World/BP_Grove` | `UTaeGroveComponent` is the root, `BoxExtent (700, 700, 200)`, `RegenCurve` assigned, compiled and saved |

**Why placement is manual:** `EditorActorSubsystem.spawn_actor_from_object` hard-crashes the
`-run=pythonscript` commandlet — access violation inside `EditorFramework.dll` immediately after
`load_level`. Actor placement wants a level-editor context the commandlet never builds. Not worth
fighting; the HUD work below needs the editor anyway.

---

## Step 1 — Place the grove in `WorldNull` -- DONE

1. Open `Content/Maps/WorldNull`.
2. Drag `Content/World/BP_Grove` into the level.
3. Set its location to **`(-1570, -4400, 100)`**.
   The starting anchor `BP_RootAnchor_C_1` sits at `(-1570, -3760, 0)`, so this is ~6.4 m south of it —
   reachable on foot, not overlapping. `Z = 100` puts the box floor at ground level, since the extent
   is a half-size of 200.
4. Save the level.

**The grove is an invisible box** — a `UBoxComponent` with no mesh. Two ways to see it:

- `show Collision` in the PIE console (testing only), or
- add a `StaticMeshComponent` child in `BP_Grove` — a Plane scaled `(14, 14, 1)` with a green-ish
  material. Worth doing before the gate clip, since a grove nobody can see makes a poor recording.
  Set it below the box so the player walks over it.

---

## Step 2 — Verify the cues registered -- DONE

This is the step that catches the one M2 failure mode static analysis can't. Cue notifies are found by
an **asset-registry scan of asset names**, so a rename silently unregisters a cue.

In the editor console (or PIE console):

```
GameplayCue.PrintGameplayCueNotifyMap
```

**Expect** one line per tag, all three mapped to an index:

| Asset | Derived tag | Expected line |
|---|---|---|
| `GC_Mana_Drain` | `GameplayCue.Mana.Drain` | `GameplayCue.Mana.Drain -> <index>` |
| `GC_Mana_Regen` | `GameplayCue.Mana.Regen` | `GameplayCue.Mana.Regen -> <index>` |
| `GC_Arcane_Exhausted` | `GameplayCue.Arcane.Exhausted` | `GameplayCue.Arcane.Exhausted -> <index>` |

`-> unmapped` or `-> index not found` means the asset name no longer derives its tag — compare against spec §6. Output is `LogAbilitySystem: Warning`, so read it in the Output Log.

**Do not use `GameplayCue.PrintLoadedGameplayCueNotifyClasses` for this.** It prints `LoadedGameplayCueNotifyClasses` (`GameplayCueManager.cpp:1264-1268`), which is only appended as notifies are *async-loaded* (`:1069`) — on a fresh editor it is empty whether or not registration succeeded, so an empty list proves nothing. `PrintGameplayCueNotifyMap` reads the runtime cue set built by the registry scan, which is the thing being verified.

If a tag does come back unmapped, **Window → Gameplay Cue Editor** shows the tag ↔ handler table directly and will say which asset (if any) claimed the tag.

RESULT (2026-08-15): **PASS.** All three registered — `Mana.Regen -> 0`, `Mana.Drain -> 1`, `Arcane.Exhausted -> 2`.

Two harmless `unmapped` lines accompany them: `GameplayCue.Mana` and `GameplayCue.Arcane` are the implicit parent tags of the hierarchy and no notify is meant to handle them. A third, `GameplayCue.Test`, belongs to the engine — it is declared by GAS's own unit tests (`GameplayEffectTests.cpp:32`) and is not ours.

An earlier attempt with the `PrintLoaded…` variant returned only the `No GameplayCueNotifyPaths were specified … Falling back to using all of /Game/` fallback warning and was mistaken for a failure. That warning is unrelated to registration; it only reports that the scan root defaulted to `/Game/`, which is where `Content/GAS/Cues/` lives.
---

## Step 3 — The HUD mana bar -- DONE

Open `Content/UI/Widgets/WBP_HUD`. The ViewModel is already registered there (the existing mana text
binds to it), so this is bindings only — **no converters**, every bound property is already in its
presentation form.

1. Add a `ProgressBar` named `ManaBar` beside the existing mana text.
2. In **View Bindings**, bind:
   - `ManaBar → Percent` ← `ManaPercent`
   - `ManaBar → Fill Color and Opacity` ← `ManaBarTint`
3. Add a `TextBlock` named `ExhaustedLabel` reading **"DEPLETED"**.
4. Bind `ExhaustedLabel → Visibility` ← `ExhaustedVisibility`.
5. Compile and save.

---

## Step 4 — Verify in PIE -- DONE

RESULT (2026-08-15): **PASS**, all ten rows, on the first run where the HUD was actually connected. Row 9 confirmed specifically: entering Arcane inside the grove stops the climb, and dropping back to Forest resumes it *without* leaving and re-entering the volume — so the `UTargetTagRequirementsGameplayEffectComponent` on `UTaeManaRegenEffect` is inhibiting rather than being removed and reapplied, which is the whole reason it is built that way.

Three defects had to be fixed before this step could run at all, none of them in M2's gameplay code:

1. `GameInstanceClass` in `Config/DefaultEngine.ini` pointed at `/Game/Blueprints/Core/BP_TaeGameInstance`, but the asset lives at `/Game/Core/BP_TaeGameInstance`. The class silently failed to load and the engine fell back to base `UGameInstance`, so `UTaeGameInstance::Init` never ran, `ATaePlayerController::SetPawn` bailed at its `GetGameInstance<UTaeGameInstance>()` null check, and no attribute ever reached the ViewModel. This predates M2 — M1's mana text never worked either, and nobody noticed because M1 was never PIE-verified.
2. `WBP_HUD` resolved its own viewmodel instance instead of the one the controller writes to. Now published to the global collection by `UTaeGameInstance::Init` under the identifier `HudViewModelContextName` (`"HudViewModel"`), and fetched by the widget via Creation Type `Global View Model Collection`. The old `Event Construct → Cast → SET` chain in the widget graph is gone.
3. The View Bindings rows were authored with source and destination swapped. In the binding row the **left** field is the destination and the **right** is the source (`SMVVMBindingRow.cpp:122` vs `:169`), so the ViewModel belongs on the right.

`showdebug abilitysystem` is the better instrument here than the HUD — it shows `Mana`, owned tags
(`Arcane.Vision`, `Arcane.Exhausted`), and active abilities together, so a drain that isn't happening
can be traced to the ability rather than the attribute.

Mana starts at `100 / 100`. Expected rates with current defaults:

| # | Do this | Expect |
|---|---|---|
| 1 | Hold Arcane Vision | Mana falls ≈ **4/s**. Bar tints orange, flow reads Draining |
| 2 | Toggle Arcane off | Fall stops. Tint returns to white |
| 3 | In Arcane, channel at an anchor | Mana falls ≈ **16/s** (4 vision + 12 growth) |
| 4 | Release the channel | Back to ≈ 4/s, growth progress preserved |
| 5 | Hold Arcane to zero | Arcane drops on its own, vignette flashes, **DEPLETED** appears |
| 6 | Press the Arcane toggle again | Nothing happens — re-entry refused |
| 7 | Walk into the grove | Mana climbs ≈ **10/s**, bar tints green |
| 8 | Watch the bar pass 25 | **DEPLETED** clears, Arcane becomes available again |
| 9 | Enter Arcane while still inside the grove | Climb **stops** — and on dropping back to Forest it resumes **without leaving and re-entering the volume** |
| 10 | Return to the anchor and finish the connection | Completes, no errors in the log |

Check **9** specifically — it is the whole reason regen uses an ongoing tag requirement instead of
being removed and reapplied. If regen only resumes after stepping out and back in, the
`UTargetTagRequirementsGameplayEffectComponent` on `UTaeManaRegenEffect` is not doing its job.

**Grove rate check:** a `(700, 700)` extent is 14 m × 14 m = 196 m², which reads ≈ **9.9/s** off
`Curve_GroveRegen`. If regen is 0, `RegenCurve` came unassigned — asset validation will say so.

---

## Step 5 — Tune (Task 12)

Editor-only, no code. Play the gate scenario end to end first, then adjust:

| Value | Where | Default |
|---|---|---|
| `ArcaneDrainPerSecond` | `BP_GA_SpectralShift` | `4` |
| `ManaCostPerSecond` | `BP_GA_GrowRoot` | `12` |
| `GrowthRate` | `BP_GA_GrowRoot` | `0.35` (a full path = ~2.9 s of channel) |
| `ExhaustionRecoveryFraction` | `BP_TaeCharacter` | `0.25` |
| Curve keys | `Curve_GroveRegen` | `50 → 4`, `200 → 10` |

Targets from the spec: surveying an island pair costs a noticeable fraction of the bar, **one full
connection cannot be grown in a single charge from full**, and a recovery visit takes a few seconds
rather than a wait.

With the defaults, a full connection costs ~2.9 s × 16/s ≈ **46 mana** — under half a bar, so it *can*
be done in one charge. Expect to raise `ManaCostPerSecond` or lower `GrowthRate`.

Record what the numbers land on in spec §5 under "Tuned values (M2 gate)".

---

## Step 6 — The gate clip

One continuous take: reveal → channel → run dry → ejected with the vignette flash → re-entry refused →
walk to the grove → recover → return → finish the connection.

---

## Report back

Scratch notes and bug artifacts go in `docs/issues/`, which is **gitignored** — it is a working area, not part of the record. Anything that turns out to matter gets promoted into this handoff, the plan, or the spec, where it is versioned. Observation first, diagnosis second.
