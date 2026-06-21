# Visual Assets — Level Dressing Suggestions

> Sourcing pass for Day 7/8 (see `docs/LevelGeneration.md`). Project license is source-available,
> portfolio/educational, **no commercial use, no redistribution** — prioritize CC0 or equally
> permissive packs below. Anything from Fab/Quixel is flagged separately; verify its license terms
> before committing the raw assets into this public repo (see note at the bottom).

## Big Set Dressing (hand-placed island rocks/ruins)

- **Quaternius — Ultimate Modular Ruins Pack** — quaternius.com/packs/ultimatemodularruins.html (also quaternius.itch.io) — CC0 — modular ruin/dungeon pieces, FBX/OBJ/Blend. *Stylized low-poly look, not photoreal — check it reads OK next to the Substrate-shaded `M_GridCube_*` materials before committing to it.*
- **Quaternius — Stylized Nature MegaKit** — quaternius.com/packs/stylizednaturekit.html — CC0 — 27 rocks + moss/grass/trees in the same pack, FBX/OBJ/glTF. Good if rocks and vegetation should share one consistent style.
- **Fab (Epic) — "Ancient Ruins" Free Asset Pack (26 pieces)** — fab.com/listings/c6bd4209-8420-499f-99b7-f47b8492b2d1 — free, custom weathered stone shader included — more detailed/realistic than Quaternius. *License flag — see bottom note.*
- **Fab (Epic) — "Rocks"** — fab.com/listings/c52ec681-084d-417b-83ad-7d91b9b7f43d — free, photo-scanned rocks (Quixel-style realism) — best match if you want photoreal rocks rather than stylized. *License flag — see bottom note.*

## Small Clutter (`ATaeClutterScatter` mesh pool — rocks, wire piles, scrap)

- **Quaternius nature packs above** — reuse the small rocks from Stylized Nature MegaKit for the rock half of the clutter pool — keeps clutter and big set dressing visually consistent.
- **Fab (Epic) — "Back Alley" industrial pack (~50 assets)** — free — trash bags, dumpsters, pipes, busted pallets, fences — good source for the "industrial junk"/scrap half of the pool. *License flag — see bottom note.*
- **CGTrader / TurboSquid free tiers — cable & scrap pile models** (e.g. TurboSquid "free Cable/Wire" search, CGTrader "Scrap"/"Junk" categories) — mixed licenses per-model, **check each model's individual license** — these are marketplaces, not a single CC0 pack; only use models explicitly marked royalty-free/CC0 for redistribution.
- **Kenney — Platformer Pack Industrial / City Kit (Industrial)** — kenney.nl/assets — CC0, no attribution — but Kenney's house style is flat-shaded low-poly stylized, a likely visual mismatch against this project's realistic Substrate materials. Treat as a fallback only if nothing else fits the budget.

## Interactables (`BP_Chest`, `BP_Pickup_Mana`)

- **KayKit — Dungeon Pack Remastered** — kaylousberg.itch.io/kaykit-dungeon-remastered — CC0, no attribution — has chest meshes among 200+ stylised dungeon props, FBX/GLTF/OBJ. Same low-poly-stylized caveat as Kenney/Quaternius — check it sits next to the rest of the art before committing.
- **KayKit — Resource Bits** — kaylousberg.itch.io/resource-bits — CC0 — small gem/resource pickups; worth checking directly for a mana-orb-style pickup mesh (not independently confirmed during this pass — verify on the page).
- **itch.io CC0 3D collections** (e.g. itch.io/game-assets/assets-cc0/tag-3d) — broader net for a crystal/orb pickup if KayKit doesn't have a good fit — filter by CC0 tag specifically, not just "free".

---

## License note — Fab / Quixel content

Fab's "Free" listings (including Epic's own giveaways like "Ancient Ruins" and "Rocks" above) are
typically licensed under Epic's **Standard/Content License**, not CC0. These generally permit use
*inside Unreal Engine projects you build*, but may restrict redistributing the raw, un-compiled
assets (the `.uasset`/source files themselves) in a separate public repository. Since this repo is
public and source-available, **read the specific license on each Fab listing page before adding its
files to this repo** — when in doubt, prefer the CC0-licensed Quaternius/KayKit packs above, which
carry no such restriction.

## Priority order to audition

1. Quaternius packs — covers big set dressing + small rocks in one consistent CC0 style, no license risk
2. KayKit Dungeon Remastered — chest + dungeon clutter, CC0, check style fit against Quaternius
3. Fab free packs — only if style/realism needs outweigh the license-check overhead
