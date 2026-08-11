# M2 — Mana Has Teeth · Design Spec

> Implements milestone M2 from [the restoration economy spec](2026-08-11-restoration-economy-design.md),
> which remains the authority on the economy as a whole. The growth mechanic itself is unchanged from
> [the connection loop spec](2026-08-10-connection-loop-design.md).
>
> Engine: UE 5.8. M1 (Connection Loop Playable) is complete and merged.

---

## 1. Summary

Arcane Vision currently costs nothing. Mana exists as an attribute, is drained only by `UGA_GrowRoot`
poking `SetNumericAttributeBase` directly, and regenerates never. Forest mode therefore has no reason to
exist — you can survey forever.

M2 gives the meter teeth: Arcane drains while active, growing drains faster, running dry ejects you to
Forest and locks re-entry until you recover, and recovery happens only on living land. It is deliberately
small and self-contained, and it makes M1's loop tense without adding any new verb.

**Mana only.** Saplings arrive whole in M3, together with the reveal that makes them findable.

---

## 2. Decisions

| # | Decision | Rejected |
|---|---|---|
| 1 | **Applied periodic Gameplay Effects** for all three flows | Ability cost-commit loop; keeping the C++ timer math |
| 2 | **Exhausted tag with a recovery floor** | Activation cost only; no re-entry gate |
| 3 | **Overlap volume with an area-driven rate** as the regen source | Distance falloff; global regen with no zone |
| 4 | **Cues route through existing systems** — no new art | Exhaustion cue only; placeholder Niagara |
| 5 | **Gameplay Effects defined in C++**, not Blueprint assets | BP GE assets authored in-editor |
| 6 | **Grove is a component**, not an actor | `ATaeGrove` standalone actor; introducing `ATaeIsland` now |
| 7 | **UE's own Python plugin** for editor automation | Third-party Unreal MCP plugin; fully manual handoff |

### Why periodic GEs rather than ability costs

`CostGameplayEffectClass` and `CommitAbilityCost` are GAS's canonical path for *one-shot* costs, and they
would give affordability checks for free. But a continuous drain still needs its own timer, so that route
ends with cost-commit and timer logic side by side — more GAS vocabulary on screen, less coherence.

Applied Infinite-with-Period effects express "this ability is costing you, right now, for as long as it
runs" directly. They are removed by handle when the ability ends, which is the same lifetime the ability
already manages.

The trade-off this creates: a periodic GE cannot report "payment failed" — it clamps at zero and keeps
running. Exhaustion therefore has to be detected from the attribute value, which is what decision 2 does
anyway.

### Why the rates are not on the effects

Decision 5 puts the GEs in C++, and all magnitudes come from **SetByCaller**. The effects hold no numbers
at all; every rate is an `EditDefaultsOnly` float on the thing that spends it (§5). This is what lets one
drain effect serve two abilities at two rates, and it satisfies the economy spec's requirement that all
rates be tunable in-editor before content is built on them.

It also means the GEs carry no tuning data, which is precisely why they do not need to be assets.

### Why Grove is a component

M4 has not yet decided whether restoration state is per-island or per-region (economy spec §11). A
`UBoxComponent` subclass can be hosted by a bare placeholder actor now and adopted by an island or region
actor later with no refactor, so the decision stays deferred where M4 put it.

Introducing an `ATaeIsland` now would pre-empt that, and the previous `ATaeIsland` was scrapped in the
2026-06-21 pivot. A component costs the same to build and is strictly more placeable.

---

## 3. Architecture

```
UTaeManaAttributeSet          the meter + the exhaustion state machine
  PreAttributeChange            clamps (exists, unchanged)
  PostAttributeChange           NEW — toggles TAG_Arcane_Exhausted with hysteresis
  EvaluateExhaustion()          NEW — static, pure, unit-tested

UTaeManaDrainEffect           NEW. Infinite, Period 0.1s, SetByCaller "Rate"
  applied by UGA_SpectralShift    rate = ArcaneDrainPerSecond
  applied by UGA_GrowRoot         rate = ManaCostPerSecond
  cue GameplayCue.Mana.Drain

UTaeManaRegenEffect           NEW. same shape, positive magnitude
  applied by UTaeGroveComponent   rate = RegenRateForArea(footprint)
  TargetTagRequirements component  ignore Arcane.Vision — inhibited, not removed
  cue GameplayCue.Mana.Regen

UTaeGroveComponent            NEW : UBoxComponent — a patch of living land
  RegenCurve                    UCurveFloat*, horizontal area (m²) -> mana/sec
  RegenRateForArea()            static, pure, unit-tested
  overlap begin/end             applies / removes the regen effect on the entering ASC

UTaeCueNotify_ManaDrain        NEW : UGameplayCueNotify_Static
UTaeCueNotify_ManaRegen        NEW : UGameplayCueNotify_Static
UTaeCueNotify_ArcaneExhausted  NEW : UGameplayCueNotify_Static
```

Both abilities apply the **same** drain effect class. Infinite GEs do not stack by default, so two
applications tick independently and the drains add up — which is the intent: growing costs vision plus
growth, not growth alone. Each ability stores its own `FActiveGameplayEffectHandle` and removes only that
one.

**Regen is suppressed while Arcane Vision is active.** Standing in a grove does not refill you mid-survey;
you have to drop back to Forest to recover, which is the pressure the whole milestone exists to create.
This is expressed as an ongoing tag requirement blocking on `Arcane.Vision` rather than by removing and
reapplying the effect, so the player never has to leave and re-enter the volume to resume regenerating.
`UGameplayEffect::OngoingTagRequirements` is deprecated in 5.8 (`GameplayEffect.h:2360`), so it goes
through `UTargetTagRequirementsGameplayEffectComponent`, created in the constructor with
`CreateDefaultSubobject` and added to `GEComponents`.

> **Corrected 2026-08-11, during implementation.** This originally said `FindOrAddComponent<T>()`. That
> helper calls `NewObject` with `NAME_None`, which is fatal inside a `UObject` constructor — it crashed the
> editor on startup. The engine's own native-GE pattern is `CreateDefaultSubobject`;
> `UGameplayEffect::PostInitProperties` ensures on *"should be added to GEComponents during the constructor
> or in PostInitProperties"* (`GameplayEffect.cpp:236`).

### Tags

New native tags in `TaeGASTypes.h`, beside `TAG_Arcane_Vision` and `TAG_Arcane_Growing`. No `FName`
strings anywhere, per the existing convention.

```
Arcane.Exhausted                 blocks and cancels Arcane Vision
GameplayCue.Mana.Drain           looping, carried by the drain effect
GameplayCue.Mana.Regen           looping, carried by the regen effect
GameplayCue.Arcane.Exhausted     burst, fired on the exhaustion transition
```

---

## 4. The exhaustion cascade

Most of this already exists; M2 adds only the first two steps.

```
Mana reaches 0
  └─> PostAttributeChange adds TAG_Arcane_Exhausted
        ├─> UGA_SpectralShift's tag listener cancels it
        │     └─> Arcane.Vision removed
        │           ├─> UGA_GrowRoot's existing listener cancels the channel  (GA_GrowRoot.cpp:107)
        │           │     └─> partial GrowthAlpha kept, as M1 already guarantees
        │           └─> ArcaneBlendAlpha targets 0 — camera and post-process fade back
        └─> GameplayCue.Arcane.Exhausted burst -> FlashVignette + HUD flash

Re-entry refused while the tag is held
  └─> UGA_SpectralShift::ActivationBlockedTags

Mana climbs back above ExhaustionRecoveryFraction * MaxMana
  └─> PostAttributeChange removes the tag — Arcane available again
```

`UGA_SpectralShift` listens for the tag rather than relying on `ActivationBlockedTags` alone, because
blocked tags prevent *activation* but do not cancel a running ability. This mirrors how `UGA_GrowRoot`
already watches `Arcane.Vision` (`GA_GrowRoot.cpp:102-114`), so the pattern is consistent rather than new.

**`UGA_GrowRoot` gains no exhaustion logic — it loses some.** The mana block at `GA_GrowRoot.cpp:126-134`
is deleted; `TickGrowth` keeps only `AdvanceGrowth` and the completion check. The channel stopping when
mana runs out becomes a consequence of the cascade rather than a rule stated twice.

### Hysteresis

`EvaluateExhaustion(Mana, MaxMana, RecoveryFraction, bWasExhausted)` returns the new exhausted state:

- Not exhausted, `Mana <= 0` → become exhausted
- Exhausted, `Mana >= RecoveryFraction * MaxMana` → recover
- Otherwise → unchanged

Stated as a pure function so the boundary behaviour is testable without an ASC, and so the two thresholds
can never drift apart in the caller.

---

## 5. Tuning surface

Every rate is `EditDefaultsOnly` on the thing that spends it. Starting values, all expected to move
during in-editor tuning before any content is built on them:

| Knob | Where | Start | Note |
|---|---|---|---|
| `ArcaneDrainPerSecond` | `UGA_SpectralShift` | 4 | ~25s of pure surveying from full |
| `ManaCostPerSecond` | `UGA_GrowRoot` | 12 | exists today; 16/s combined while channelling |
| `ExhaustionRecoveryFraction` | `ATaeCharacter` | 0.25 | pushed to the attribute set |
| `RegenCurve` | `UTaeGroveComponent` | 50 m² → 4/s, 200 m² → 10/s, capped | authored curve asset |

`ExhaustionRecoveryFraction` lives on the character rather than the attribute set because attribute sets
are not editable in the Blueprint details panel. `ATaeCharacter::PostInitializeComponents` hands the value
to the set, making `BP_TaeCharacter` the single editable surface.

Area is computed in **square metres**, not `uu²`, so authored curve values stay legible.

---

## 6. Cues

Three cues, all implemented against systems that already exist. No new art, no audio — the audio pass is
M5.

| Cue | Type | Attached to | Drives |
|---|---|---|---|
| `GameplayCue.Mana.Drain` | Looping | drain effect | ViewModel draining state → HUD bar tint |
| `GameplayCue.Mana.Regen` | Looping | regen effect | ViewModel regenerating state → HUD bar tint |
| `GameplayCue.Arcane.Exhausted` | Burst | exhaustion transition | `UTaeArcaneSubsystem::FlashVignette` + HUD flash |

Cues are attached through each effect's `GameplayCueTags`, so the routing is GAS's own rather than
hand-rolled dispatch. That routing is the thing worth demonstrating; the visuals it drives are
deliberately modest.

The looping drain cue fires once per active drain effect, so channelling adds a second instance. The
notify must therefore be idempotent — it sets a state, it does not toggle one.

A static notify is stateless and receives only the target actor, so it reaches the UI the same way
everything else does: target → `UTaeGameInstance` → `UTaeHudViewModel`. The cue never touches a widget
directly.

### Cue registration is asset-driven — resolved 2026-08-11

Checked against the engine rather than assumed. `UGameplayCueManager` builds its global cue set from an
**asset registry scan** (`GameplayCueManager.cpp:830-896`), keyed on the `AssetRegistrySearchable`
`GameplayCueName` field. Pure C++ notify classes are not assets, so they are never discovered.

There is a second trap. `UAbilitySystemGlobals::DeriveGameplayCueTagFromClass`
(`AbilitySystemGlobals.h:123-145`) special-cases a child whose tag merely equals its parent's: it clears
the tag, tries to derive one from the asset name, and on failure restores the parent tag **and returns
early, leaving `GameplayCueName` as `None`**. A Blueprint child that inherits its tag from a C++ parent is
therefore registered under nothing.

So the tag must derive from the **asset name**. `DeriveGameplayCueTagFromAssetName`
(`AbilitySystemGlobals.cpp:135-150`) strips a `GC_` prefix, replaces `_` with `.`, prepends `GameplayCue.`
if absent, and requires the result to be an already-registered tag:

| Asset | Derives | Requires native tag |
|---|---|---|
| `GC_Mana_Drain` | `GameplayCue.Mana.Drain` | ✓ declared in `TaeGASTypes.h` |
| `GC_Mana_Regen` | `GameplayCue.Mana.Regen` | ✓ |
| `GC_Arcane_Exhausted` | `GameplayCue.Arcane.Exhausted` | ✓ |

**Consequences, all binding:**

- The C++ notify classes must leave `GameplayCueTag` **unset**, so derivation runs from the child's name.
- Each cue needs a thin Blueprint asset in `Content/GAS/Cues/`, named exactly as above. This is the same
  C++-logic / BP-asset split the project already uses for `UGA_SpectralShift` → `BP_GA_SpectralShift`.
- Overrides are `const` (`OnActive_Implementation` and friends are `BlueprintPure`), which enforces the
  stateless, idempotent notify the looping cue needs anyway.
- Verify with the `GameplayCue.PrintLoadedGameplayCueNotifyClasses` console command.

---

## 7. UI

`UTaeHudViewModel` currently exposes `Mana` and `ManaText` only (`TaeHudViewModel.h:19-31`). M2 extends it,
keeping the established raw/presentation split:

| Field | Kind | Purpose |
|---|---|---|
| `MaxMana` | raw | bar range |
| `ManaPercent` | raw | bar fill |
| `bExhausted` | raw | logic |
| `ManaBarTint` | presentation | drain / regen / exhausted state, bindable with no converter |
| `ExhaustedVisibility` | presentation | exhaustion warning |

`ATaePlayerController::SetPawn` gains two bindings beside the two it already has
(`TaePlayerController.cpp:79-95`): the `MaxMana` attribute change delegate, and the `Arcane.Exhausted` tag
event.

---

## 8. Testing

Two pure statics with automation tests, following the precedent already set by
`UTaeArcaneSubsystem::StepBlendAlpha` and `TaeConnectionTypes` in `Private/Tests/`:

| Function | Covers |
|---|---|
| `UTaeManaAttributeSet::EvaluateExhaustion` | both thresholds, the hysteresis band, and that neither boundary flickers |
| `UTaeGroveComponent::RegenRateForArea` | area → rate through the curve, including a null-curve guard |

The cascade in §4 is integration behaviour and is covered by the gate clip, not by a unit test — it spans
an ASC, two abilities, and a subsystem, and mocking that costs more than it proves.

---

## 9. Editor automation

`PythonScriptPlugin` ships with 5.8 but is not enabled in `ThroughArcaneEyes.uproject`. M2 enables it.
Scripts live in `Tools/Python/`.

This is why decision 5 matters: with the Gameplay Effects in C++, the editor handoff shrinks to asset
creation and placement, which Python does well — the three `GC_*` cue Blueprints required by §6, the
`BP_Grove` actor, the regen curve asset, placing the grove in `WorldNull.umap`, saving. None of these
carry Blueprint graph logic; they are empty subclasses and placed instances.

**One step stays manual: the `WBP_HUD` mana bar.** Adding a progress bar and wiring its MVVM bindings is
designer-surface work that Python handles badly. This is the whole residue of the M1-style handoff.

Third-party Unreal MCP plugins were considered and deferred. They add live editor control including
Blueprint graph editing, but M2's Blueprints are configuration rather than logic, so the capability buys
nothing here. Revisit when something genuinely needs it — the M3 Slate overlay and M5 Control Rig are more
plausible triggers.

---

## 10. Gate

One recordable clip:

> Enter Arcane — mana visibly drains. Channel at a break — the drain accelerates. Run dry — the exhaustion
> flash fires and Arcane drops away on its own, with partial growth kept. Re-entry is refused. Walk to the
> grove — mana climbs. Re-enter and finish the connection.

---

## 11. Out of scope

| Deferred to | Item |
|---|---|
| M3 | Saplings, the reveal, persistent marks, the Slate network overlay |
| M4 | PCG re-run on restoration, real regen zones replacing the placeholder grove |
| M5 | Save/load, audio, Control Rig, win condition |

Docs updated in the same pass: README vocabulary gains `Grove` and `Arcane.Exhausted`; `AGENTS.md`'s
naming table gains a Gameplay Cue row (`GC_` / `UTaeCueNotify_`) and a note on `Tools/Python/`.

---

## 12. Risks

| Risk | Mitigation |
|---|---|
| ~~C++ CDO Gameplay Cue registration~~ | **Resolved** — checked against engine source; cue discovery is asset-registry driven, so thin BP assets named `GC_*` are required, not optional. See §6 |
| Mana tuning is genuinely hard: two drains that stack, one regen source | Every rate `EditDefaultsOnly` (§5); tune in-editor before M3 content is built on top. The gate clip is the tuning target |
| Two drain effects active at once may behave unexpectedly under GAS stacking rules | Default Infinite GEs do not stack; confirm both handles tick independently early, since the whole cost model rests on it |
| The recovery floor could strand a player far from the grove with no way back | Acceptable in M2 — one grove, one island pair, short distances. M4's real regen zones remove the condition. Revisit only if the gate clip shows it |
