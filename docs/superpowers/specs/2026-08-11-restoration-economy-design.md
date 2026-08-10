# Restoration Economy — Design Spec

> Defines the resource economy and core loop for Through Arcane Eyes, and restructures milestones M2–M5
> around it. Builds on [the connection loop spec](2026-08-10-connection-loop-design.md), which remains
> the authority on the growth mechanic itself.
>
> Engine: UE 5.8. M1 (Connection Loop Playable) is complete and merged.

---

## 1. Summary

Ant explores a dead world looking for signs of life, then spends those signs reconnecting it. The world
that gets restored is what pays for restoring more of it — resources are not farmed, they are *grown by
your own progress*.

Two resources. Two verbs: **Look** and **Plant**.

---

## 2. Decisions

| # | Decision | Rejected |
|---|---|---|
| 1 | **Two resources — Mana and Saplings** | Three (adding Sap); one (saplings only) |
| 2 | **Restoration re-runs PCG** — the world heals and produces | Hand-placed resources; dormant sources awakening |
| 3 | **Arcane reveals saplings, the mark persists** | Arcane-only visibility; visible in both modes |
| 4 | **Keep hold-to-channel; invest in the spectacle** | Directed growth puzzle; sapling types |
| 5 | **Small economy milestone first, then restructure** | One big restructure; append economy to M5 |

### Why two resources, not one

On simplicity alone one and two are close. On *demonstrable skill* two wins: a second resource is the
legitimate reason to use GAS properly. The project currently spends mana with `SetNumericAttributeBase`
— poking the attribute directly, which reads as "used GAS as an attribute bag." An economy justifies
cost Gameplay Effects, periodic regen Gameplay Effects, and Gameplay Cues. One resource gives none of
that anything to hang on.

### Why no Sap

Gathering is a familiar system that costs real implementation time and demonstrates little. A third
currency would also split growing from seeing, which would make Arcane a lens you flick on rather than a
place you spend time — undoing the pressure that gives Forest mode its purpose.

---

## 3. The Loop

```
FOREST — free, safe, blind
  travel on roots already grown
        │  toggle Arcane (mana starts draining)
        ▼
ARCANE — expensive, revealing
  saplings glow through geometry · broken connections show
  mark what you found, drop back out
        │
        ▼
FOREST — walk to the mark, collect the sapling for free
        │
        ▼
ANCHOR — plant sapling, enter Arcane, hold to grow
  mana drains faster while channelling
        │
        ▼
RESTORED — the region's PCG re-runs
  vegetation spawns · rust recedes · new saplings appear
  the region becomes a mana regen zone
        │
        └────────► you can afford to explore again
```

**The loop closes on itself.** You never farm; you heal, and healing produces.

---

## 4. Resources

### Mana — one meter, three jobs

| Job | Behaviour |
|---|---|
| Arcane Vision | Drains slowly while active |
| Growing | Drains faster while channelling |
| Recovery | Regenerates only near restored land |

Mana is the pressure that makes Forest mode necessary. Without a cost on Arcane, Forest has no reason to
exist — which is the current state of the build and the hole this closes.

**Regen scales with the extent of what was restored.** A large restored island pays more than a small
one. This gives tuning a natural axis rather than a magic constant, and it rewards the *scale* of a
restoration rather than merely counting them.

To be unambiguous: each restored region contributes a regen rate derived from **the horizontal area of
its restoration bounds**, mapped through a designer-authored curve so the relationship need not be
linear and can be capped. Not segment count, not connection count — area.

All rates are `EditDefaultsOnly` and must be tuned in-editor before content is built on top of them.

**M2 ships before restoration exists.** Until M4, regen comes from a single hand-placed placeholder zone
so the economy is tunable ahead of the system that will eventually drive it.

### Saplings — not currency

Each sapling is a specific living thing that was found, not a unit of stock. One sapling plants one
connection. They gate **where** you can grow, never **how much**.

- Invisible in Forest mode.
- Revealed in Arcane as points of life visible through geometry.
- **The mark persists** after dropping back to Forest, so travel to it is free.

This is what gives each mode a distinct job: **Arcane is the expensive scanning verb, Forest the cheap
travelling verb.** The player toggles deliberately rather than constantly.

---

## 5. Restoration

Completing a connection flips its region's restoration state and re-executes that region's PCG graph
with a `bRestored` parameter. Vegetation spawns, rust recedes, new saplings appear where there was
nothing, and the region begins granting mana regen.

This single mechanic does four jobs at once:

1. Demonstrates PCG at runtime — a marquee UE5 feature
2. Makes M4 load-bearing rather than decorative set dressing
3. Delivers the emotional payoff the game is named for
4. **Is** the resource-regeneration answer, mechanically rather than metaphorically

Without it, the current build allows roughly three connections before mana is exhausted permanently.

---

## 6. Growing — unchanged verb, transformed presentation

The hold-to-channel mechanic from M1 stays exactly as built and verified. All effort goes into how the
root *materialises*:

- **Geometry Script** — mesh generated and thickened at runtime rather than pre-made spline meshes
- **Niagara** — tendrils questing ahead of the growth front, spores, sap motes
- **Materials** — bark spreading along the spline, driven by the existing `GrowthAlpha`
- **Control Rig** — the sapling unfurling as it takes *(the rig is authored in M5 alongside Ant's tree
  locomotion; growth presentation before then uses materials and Niagara only)*

The technical showcase lives in the rendering, not the input scheme. Runtime procedural geometry
demonstrates considerably more than a waypoint minigame would, and it costs no rework of proven code.

---

## 7. Milestones (restructured)

> This spec describes a program, not a single implementation plan. **M2 is the first plan unit**; each
> later milestone gets its own plan once its predecessor's gate is met. M3–M5 are deliberately less
> detailed here — they will be re-specified against what M2 and M4 actually teach.

| Milestone | Delivers |
|---|---|
| **M2 — Mana Has Teeth** | Cost GE on Arcane, periodic regen GE, Gameplay Cues, MVVM resource binding. Small and self-contained; makes M1's loop tense immediately |
| **M3 — Arcane As A Sense** | Slate network overlay **and** sapling reveal with persistent marks — one reveal system, two uses |
| **M4 — The World Heals** | PCG re-runs on restoration; vegetation and resources spawn; regen zones activate |
| **M5 — Progression & Polish** | Win condition, save/load, audio, Control Rig tree motion |

**Control Rig slips to M5.** Ant's gait matters less than the loop having pressure. This supersedes the
M2–M5 ordering in the connection loop spec.

**The sapling reveal and the network overlay are the same technology** — "Arcane shows you things through
geometry, and the mark persists." Building them in separate milestones would duplicate the work, which is
why they share M3.

---

## 8. UE Showcase Mapping

**Programmer**

| Feature | Where |
|---|---|
| GAS cost Gameplay Effects | M2 — replaces direct `SetNumericAttributeBase` |
| Periodic regen Gameplay Effects | M2 — applied on entering restored land |
| Gameplay Cues | M2 — spend/restore feedback |
| MVVM | M2 — extend the existing ViewModel to resources |
| Custom Slate widget | M3 — network overlay |
| Runtime PCG | M4 |
| Save/load of world state | M5 |

**Tech art**

| Feature | Where |
|---|---|
| Geometry Script (runtime mesh) | Growth presentation |
| Niagara | Growth front, sapling glow, spores |
| Dynamic material instances / MPC | Restoration state changes |
| Runtime Virtual Textures | Vegetation blending as land heals |
| Control Rig | M5 — tree locomotion, then sapling unfurl |

---

## 9. Cut from the source ideas

Each of these is a reasonable idea and each is a second system. Recorded so the decision is not
re-litigated by accident.

| Cut | Why |
|---|---|
| **Sap** as a third currency | Splits growing from seeing; gathering demonstrates little |
| **Memory / Resonance** meta-progression | A fourth currency for a slice that has no meta layer yet |
| **Sapling types** (vine / root / symbiotic / ancient) | Every type needs its own mesh and VFX; doubles the art cost of the money shot |
| **Five progression archetypes** (A–E) | Content design, not system design; premature before one island works |
| **Environmental restoration puzzles** | A separate mechanic wearing the economy's clothes |

---

## 10. Risks

| Risk | Mitigation |
|---|---|
| **Runtime PCG regeneration may be impractical.** Only verified that PCG ships production-ready in 5.8, not that re-running a graph at runtime is viable | Prove it early in M4 with a throwaway test, not late. Fallback: authored "dormant sources awaken" placement, which delivers the same beat without runtime generation |
| Mana tuning is genuinely hard — three drains, one regen source, all interacting | Every rate `EditDefaultsOnly`; tune before building content on top. Regen scaling with restored extent gives a principled axis rather than a magic number |
| M2's regen has nothing to regenerate *from* until M4 | M2 ships with a placeholder regen zone so the loop is tunable before PCG exists |

---

## 11. Deferred to their milestones

- How a marked sapling is displayed once the player leaves Arcane (M3)
- The physical radius and shape of a regen zone (M4)
- Whether restoration state is per-island or per-region (M4)
