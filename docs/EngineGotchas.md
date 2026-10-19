# Engine Gotchas (UE 5.8)

Pitfalls this project already hit and confirmed against engine source. Check here before re-deriving any of them.

## Framework

- If `GameInstanceClass` in `DefaultEngine.ini` points at a path that fails to load, UE **silently** falls back to base `UGameInstance` — no error, `UTaeGameInstance::Init` just never runs. Downstream symptom: `ATaePlayerController::SetPawn` bails at its null check and the HUD never updates.
- Headless boot (see [AGENTS.md](../AGENTS.md) Build & Test) runs `GameInstance::Init` and world BeginPlay — the fastest way to catch wiring bugs like the one above.
- UBT refuses to build with "Unable to build while Live Coding is active" if the editor is open. Live Coding cannot pick up **new** source files at all.

## MVVM

- The default `FMVVMBlueprintViewModelContext::CreationType` is **`CreateInstance`**, so a widget silently makes its own viewmodel unless told otherwise. `WBP_HUD` must use Creation Type `Global View Model Collection` with the name `UTaeGameInstance::HudViewModelContextName`.
- In a View Bindings row the **left** field is the Destination and the **right** is the Source (`SMVVMBindingRow.cpp:122` vs `:169`) — the `←` arrow is correct for a normal binding.
- A setter is generated only for Creation Type `Manual` or with "Create Public Setter" ticked (`MVVMViewBlueprintCompiler.cpp:944`); otherwise a Blueprint `SET` node on the viewmodel property fails to compile.
- `UMVVMSubsystem::GetGlobalViewModelCollection` is deprecated — use `UMVVMGameSubsystem::GetViewModelCollection()`.
- A widget must have **Is Variable** ticked or its generated property lacks `CPF_BlueprintVisible` (`WidgetBlueprintCompiler.cpp:1550`) and MVVM rejects it.

## GAS

- `EGameplayModOp::Additive` is a hidden back-compat alias — use `AddBase`.
- `UGameplayEffect::OngoingTagRequirements` is deprecated — use `UTargetTagRequirementsGameplayEffectComponent`. Create it with `CreateDefaultSubobject` and push it into `GEComponents`; **`FindOrAddComponent<T>()` in a constructor crashes the editor at startup** (it calls `NewObject` with `NAME_None`).
- The tag-requirements component *inhibits* an effect while requirements fail; it is not removed, so it resumes when they pass again.
- Periodic modifiers apply per *period*, not per second.
- Gameplay Cue notifies are discovered by an asset-registry scan, so pure C++ notify classes are never registered. Each cue needs a BP asset whose *name* derives the tag (`GC_Mana_Drain` → `GameplayCue.Mana.Drain`). A child inheriting its parent's tag leaves the searchable `GameplayCueName` as `None`.

## Editor Python scripting

- `UCurveFloat::FloatCurve` is a plain `UPROPERTY()` with no edit specifiers, so `set_editor_property` cannot reach it — import keys from a temporary CSV via `CSVImportFactory` instead.
- `EditorActorSubsystem.spawn_actor_from_object` **crashes** the `-run=pythonscript` commandlet (access violation in `EditorFramework.dll` after `load_level`). Actor placement needs a level-editor context the commandlet never builds — place actors by hand.
- Adding components to a Blueprint from Python works, via `SubobjectDataSubsystem` + `make_new_scene_root`.
- Niagara emitter graphs are not exposed to Python — `NS_` assets are authored by hand.

## Plugin maturity (checked by folder)

`UAF/*` (AnimNext lineage), `Mover`, `MoverAnimNext` and `PCGBiomeCore` live under `Engine/Plugins/Experimental/`. `PCG` core, `PoseSearch`, `GameplayCameras`, `GeometryScripting`, `Chooser` and `MotionWarping` are production.
