# Arcane Presentation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Give M1's root growth and M2's grove a visible presentation, and put the arcane colours in one place that materials, VFX, and M3's Slate overlay all read from.

**Architecture:** C++ owns data and hooks; the Niagara systems are authored in the editor against a documented user-parameter contract. A `UTaeArcanePalette` data asset is the single source of truth for arcane colour, copied into a Material Parameter Collection at `OnWorldBeginPlay` because Slate cannot read an MPC but can read a data asset. Two Niagara components — one on the grove, one tracking the root's growing tip — are spawned at runtime and fed parameters from state that already exists.

**Tech Stack:** UE 5.8 C++, Niagara, Material Parameter Collections, UE Automation Tests, `PythonScriptPlugin` for editor asset work.

**Spec:** [`docs/superpowers/specs/2026-08-15-arcane-presentation-design.md`](../specs/2026-08-15-arcane-presentation-design.md)

## Global Constraints

- Engine: UE 5.8 at `D:\EpicGames\UE_5.8`. Project: `D:\PetProjects\ThroughArcaneEyes`.
- Single runtime module `ThroughArcaneEyes`. **This pass adds one engine dependency — `Niagara` — in Task 2.** No new modules are created.
- Every file starts with: `// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.`
- Naming: `A`+`Tae` actors, `U`+`Tae` objects/components, `F` structs, `E` enums. API macro `THROUGHARCANEEYES_API`.
- **All `UPROPERTY` members use `TObjectPtr<T>`, never raw `T*`.** Forward-declare in headers, include in `.cpp`.
- Logging: `LogTae` only, never `LogTemp`. `Warning` for null-guards only, no flow logging.
- Prefer `IsDataValid` under `#if WITH_EDITOR` over runtime null-guards for BP-assigned properties. Include `Misc/DataValidation.h` in the `.cpp`.
- Source mirrors headers: `Public/<Domain>/TaeX.h` → `Private/<Domain>/TaeX.cpp`.
- **Niagara user-parameter names live in C++ as `FName` constants.** `SetVariableFloat` and friends fail silently on an unknown name, so both sides must match one spelling.
- Commit format: `[TAG][sigil] short description`. Sigils `[+]` add, `[-]` remove, `[*]` fix/tweak. Tags used here: `[Core]`, `[World]`, `[GAS]`, `[Config]`, `[Docs]`.
- **Markdown docs are not hard-wrapped.** One line per paragraph; the viewer soft-wraps.

**Build command** (run after every code change):

```powershell
& 'D:\EpicGames\UE_5.8\Engine\Build\BatchFiles\Build.bat' ThroughArcaneEyesEditor Win64 Development -Project="D:\PetProjects\ThroughArcaneEyes\ThroughArcaneEyes.uproject" -WaitMutex
```

Expected on success: `Result: Succeeded`.

**The editor must be closed to build.** UBT refuses with `Unable to build while Live Coding is active`. Ctrl+Alt+F11 does not help when a **new** source file has been added — Live Coding cannot pick up new files.

**Test command:**

```powershell
& 'D:\EpicGames\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' "D:\PetProjects\ThroughArcaneEyes\ThroughArcaneEyes.uproject" -ExecCmds="Automation RunTests ThroughArcaneEyes; Quit" -unattended -nopause -nullrhi -nosplash -log
Select-String -Path "D:\PetProjects\ThroughArcaneEyes\Saved\Logs\ThroughArcaneEyes.log" -Pattern "Found \d+ automation tests|Test Completed" | ForEach-Object { $_.Line }
```

**Two things that will waste your time if you skip them:**

1. **Results do not go to stdout.** Every test result lands in `Saved/Logs/ThroughArcaneEyes.log`, which is why the `Select-String` line is part of the command, not optional.
2. **The engine prints `Result={Success}`, not `Result={Passed}`.** A failing test prints `Result={Fail}`.

Baseline entering this plan: **9 tests**, all `Success` — `Core.BlendAlpha`, `GAS.ManaDrainRate`, `GAS.ManaDrainStacking`, `GAS.ManaEffects`, `GAS.ManaExhaustion`, `Harness.Sanity`, `UI.ManaFlow`, `World.GroveRegen`, `World.GrowthStep`. This plan adds two, ending at 11.

**Verifying runtime behaviour without the editor** — useful for anything that only happens at `BeginPlay`:

```powershell
& 'D:\EpicGames\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' "D:\PetProjects\ThroughArcaneEyes\ThroughArcaneEyes.uproject" /Game/Maps/WorldNull -game -nullrhi -unattended -nosplash -log -ExecCmds="quit"
Select-String -Path "D:\PetProjects\ThroughArcaneEyes\Saved\Logs\ThroughArcaneEyes.log" -Pattern "LogTae" | ForEach-Object { $_.Line }
```

## Engine API notes (verified against 5.8 source — do not substitute from memory)

- `UMaterialParameterCollectionInstance::SetVectorParameterValue(FName, const FLinearColor&)` **returns `bool` — `false` when the parameter name is not in the collection** (`MaterialParameterCollectionInstance.h:40`). This is the whole reason the apply function returns a count instead of `void`: a typo'd or missing parameter is otherwise silent.
- `UMaterialParameterCollectionInstance::GetVectorParameterValue(FName, FLinearColor&)` returns `bool` the same way (`:46`).
- `UWorld::GetParameterCollectionInstance(const UMaterialParameterCollection*)` returns the world's instance for a collection (`World.h:3317`). There is no instance without a world, which is why the palette test needs one.
- `FCollectionVectorParameter` has `ParameterName` (inherited) and `DefaultValue` (`FLinearColor`) (`MaterialParameterCollection.h:59-71`). Parameters must exist in the collection before a value can be set, so a runtime-built test collection has to populate `VectorParameters` first.
- `UNiagaraComponent::SetVariableFloat(FName, float)`, `SetVariableBool(FName, bool)`, `SetVariableVec3(FName, FVector)` (`NiagaraComponent.h:533, 551, 506`).
- `UNiagaraFunctionLibrary::SpawnSystemAttached(UNiagaraSystem*, USceneComponent*, FName, FVector, FRotator, EAttachLocation::Type, bool bAutoDestroy, bool bAutoActivate, ENCPoolMethod, bool bPreCullCheck)` (`NiagaraFunctionLibrary.h:96`). It is marked `UnsafeDuringActorConstruction`, so it must be called from `BeginPlay`, never a constructor.

---

## File Structure

| File | Responsibility |
|---|---|
| `Private/Tests/TaeTestWorld.h` | **Create.** `FScopedTestWorld`, extracted from the drain test so two tests can share it. |
| `Private/Tests/TaeManaDrainRateTest.cpp` | **Modify.** Use the extracted header. |
| `ThroughArcaneEyes.Build.cs` | **Modify.** Add `Niagara`. |
| `Public/Core/TaeArcanePalette.h` | **Create.** The data asset and the MPC parameter-name constants. Header only, no `.cpp`. |
| `Public/Core/TaeGameInstance.h` / `.cpp` | **Modify.** Hold the palette and collection, expose getters. |
| `Public/Core/TaeArcaneSubsystem.h` / `.cpp` | **Modify.** `ApplyPaletteToCollection` plus the `OnWorldBeginPlay` call. |
| `Private/Tests/TaeArcanePaletteTest.cpp` | **Create.** |
| `Public/World/TaeGroveComponent.h` / `.cpp` | **Modify.** Bloom system, three user parameters, validation. |
| `Public/World/TaeRootPath.h` / `.cpp` | **Modify.** `GrowthFrontDistance`, `SetGrowing`, front component. |
| `Private/Tests/TaeGrowthFrontTest.cpp` | **Create.** |
| `Public/GAS/GA_GrowRoot.h` / `.cpp` | **Modify.** Call `SetGrowing` at both ends of the channel. |
| `Tools/Python/tae_vfx_assets.py` | **Create.** Scripts `MPC_Arcane` and `DA_ArcanePalette`. |
| `docs/issues/2026-08-15-arcane-presentation-handoff.md` | **Create.** The editor steps. Note `docs/issues/` is gitignored — this one is deliberate, see Task 6. |

---

## Task 1: Share the test world

`FScopedTestWorld` currently lives inside `TaeManaDrainRateTest.cpp`. Task 3's palette test is its second consumer, so it moves to a header now rather than being copy-pasted. Pure refactor: no behaviour changes and the suite must stay at 9 green.

**Files:**
- Create: `Source/ThroughArcaneEyes/Private/Tests/TaeTestWorld.h`
- Modify: `Source/ThroughArcaneEyes/Private/Tests/TaeManaDrainRateTest.cpp`

**Interfaces:**
- Consumes: nothing.
- Produces: `Tae::Test::FScopedTestWorld` with a public `UWorld* World` member, and `Tae::Test::TickWorld(UWorld*, float Seconds)`.

- [ ] **Step 1: Create the shared header**

Create `Source/ThroughArcaneEyes/Private/Tests/TaeTestWorld.h`:

```cpp
// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

#if WITH_DEV_AUTOMATION_TESTS

// A real UWorld for tests that need one. Some behaviour cannot be reached by a pure function —
// periodic Gameplay Effects need a ticking world, and a Material Parameter Collection has no
// instance without one.
namespace Tae::Test
{
	// Mirrors the engine's own GAS tests (GameplayEffectTests.cpp:761): sub-tick in fixed steps so
	// periodic effects land, and bump GFrameCounter because GAS caches per-frame state.
	inline void TickWorld(UWorld* World, float Seconds)
	{
		constexpr float Step = 0.1f;
		while (Seconds > 0.f)
		{
			World->Tick(ELevelTick::LEVELTICK_All, FMath::Min(Seconds, Step));
			Seconds -= Step;
			GFrameCounter++;
		}
	}

	// Setup and teardown matching the engine's GAS tests (GameplayEffectTests.cpp:849, 865) so the
	// test leaves no world context behind.
	struct FScopedTestWorld
	{
		FScopedTestWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, false);
			FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
			WorldContext.SetCurrentWorld(World);

			FURL URL;
			World->InitializeActorsForPlay(URL);
			World->BeginPlay();

			InitialFrameCounter = GFrameCounter;
		}

		~FScopedTestWorld()
		{
			GFrameCounter = InitialFrameCounter;
			World->EndPlay(EEndPlayReason::Quit);
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		UWorld* World = nullptr;
		uint64 InitialFrameCounter = 0;
	};
}

#endif
```

- [ ] **Step 2: Point the drain test at the header**

In `Source/ThroughArcaneEyes/Private/Tests/TaeManaDrainRateTest.cpp`, add to the includes:

```cpp
#include "Tests/TaeTestWorld.h"
```

Then **delete** these three things from the `TaeDrainTest` namespace, because they now live in the header: the `TickWorld` function, the `FScopedTestWorld` struct, and the now-unused `#include "Engine/Engine.h"` / `#include "Engine/World.h"` lines.

Keep `Tolerance`, `SettleSeconds`, `MakeCaster`, `GetMana`, and `ApplyDrain` — those are drain-specific and stay.

Add this line inside the `TaeDrainTest` namespace so the existing call sites keep compiling unchanged:

```cpp
	using Tae::Test::FScopedTestWorld;
	using Tae::Test::TickWorld;
```

- [ ] **Step 3: Build and run the full suite**

Run the build command, then the test command.
Expected: `Result: Succeeded`, `Found 9 automation tests`, nine `Result={Success}` lines, no `Result={Fail`. This is a refactor — the count and the results must be identical to the baseline.

- [ ] **Step 4: Commit**

```powershell
git add Source/ThroughArcaneEyes/Private/Tests/TaeTestWorld.h Source/ThroughArcaneEyes/Private/Tests/TaeManaDrainRateTest.cpp
git commit -m "[GAS][*] share the scoped test world between tests"
```

---

## Task 2: The palette asset and the Niagara dependency

The data asset and the parameter-name constants. No behaviour yet — this task exists so Task 3 has types to consume, and it is where the `Niagara` dependency lands so every later task compiles.

**Files:**
- Modify: `Source/ThroughArcaneEyes/ThroughArcaneEyes.Build.cs:18`
- Create: `Source/ThroughArcaneEyes/Public/Core/TaeArcanePalette.h`

**Interfaces:**
- Consumes: nothing.
- Produces: `UTaeArcanePalette` with four `FLinearColor` properties — `SpectralEdge`, `CubeTint`, `GroveBloom`, `GrowthFront`; and the `TaeArcaneParams` namespace holding four matching `FName` constants.

- [ ] **Step 1: Add the Niagara dependency**

In `Source/ThroughArcaneEyes/ThroughArcaneEyes.Build.cs`, after the UI line at `:18`:

```csharp
		// VFX
		PublicDependencyModuleNames.AddRange(new string[] { "Niagara" });
```

- [ ] **Step 2: Write the palette header**

Create `Source/ThroughArcaneEyes/Public/Core/TaeArcanePalette.h`:

```cpp
// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TaeArcanePalette.generated.h"

// Names of the vector parameters in MPC_Arcane. Setting a parameter the collection does not declare
// fails silently, so the collection asset must declare exactly these four.
namespace TaeArcaneParams
{
	inline const FName SpectralEdge(TEXT("SpectralEdge"));
	inline const FName CubeTint(TEXT("CubeTint"));
	inline const FName GroveBloom(TEXT("GroveBloom"));
	inline const FName GrowthFront(TEXT("GrowthFront"));
}

// The arcane look in one place. Materials read these through MPC_Arcane, the Niagara systems sample
// the same collection, and M3's Slate overlay will read this asset directly — Slate cannot read a
// Material Parameter Collection, which is why the data asset is the source of truth and the
// collection is a derived copy.
//
// Assign DA_ArcanePalette in BP_TaeGameInstance.
UCLASS(BlueprintType)
class THROUGHARCANEEYES_API UTaeArcanePalette : public UDataAsset
{
	GENERATED_BODY()

public:
	// Edge glow on M_SpectralEdge, and the line colour of the M3 network overlay
	UPROPERTY(EditDefaultsOnly, Category = "Arcane")
	FLinearColor SpectralEdge = FLinearColor(0.2f, 0.8f, 1.f, 1.f);

	// Arcane-mode tint on M_GridCube_Arcane
	UPROPERTY(EditDefaultsOnly, Category = "Arcane")
	FLinearColor CubeTint = FLinearColor(0.1f, 0.4f, 0.6f, 1.f);

	// NS_GroveBloom — living land, so this one is deliberately warm against the cold arcane palette
	UPROPERTY(EditDefaultsOnly, Category = "Arcane")
	FLinearColor GroveBloom = FLinearColor(0.4f, 0.9f, 0.35f, 1.f);

	// NS_GrowthFront — the growing tip
	UPROPERTY(EditDefaultsOnly, Category = "Arcane")
	FLinearColor GrowthFront = FLinearColor(0.6f, 1.f, 0.5f, 1.f);
};
```

- [ ] **Step 3: Build**

Run the build command.
Expected: `Result: Succeeded`. Nothing consumes the header yet, so there is nothing to test — Task 3 adds the first behaviour and the first test.

- [ ] **Step 4: Commit**

```powershell
git add Source/ThroughArcaneEyes/ThroughArcaneEyes.Build.cs Source/ThroughArcaneEyes/Public/Core/TaeArcanePalette.h
git commit -m "[Core][+] add the arcane palette asset and the niagara dependency"
```

---

## Task 3: Populate the material parameter collection

The palette becomes real: written into an MPC at world start so every material samples one source. The write returns a count rather than `void`, because `SetVectorParameterValue` returns `false` on an unknown name and a miscounted write is exactly the silent failure worth catching.

**Files:**
- Modify: `Source/ThroughArcaneEyes/Public/Core/TaeGameInstance.h`
- Modify: `Source/ThroughArcaneEyes/Private/Core/TaeGameInstance.cpp`
- Modify: `Source/ThroughArcaneEyes/Public/Core/TaeArcaneSubsystem.h`
- Modify: `Source/ThroughArcaneEyes/Private/Core/TaeArcaneSubsystem.cpp`
- Create: `Source/ThroughArcaneEyes/Private/Tests/TaeArcanePaletteTest.cpp`

**Interfaces:**
- Consumes: `UTaeArcanePalette`, `TaeArcaneParams` (Task 2); `Tae::Test::FScopedTestWorld` (Task 1).
- Produces: `static int32 UTaeArcaneSubsystem::ApplyPaletteToCollection(UWorld* World, const UTaeArcanePalette* Palette, UMaterialParameterCollection* Collection)` returning the number of parameters written; `UTaeGameInstance::GetArcanePalette()` and `GetArcaneCollection()`.

- [ ] **Step 1: Write the failing test**

Create `Source/ThroughArcaneEyes/Private/Tests/TaeArcanePaletteTest.cpp`:

```cpp
// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "Misc/AutomationTest.h"
#include "Core/TaeArcanePalette.h"
#include "Core/TaeArcaneSubsystem.h"
#include "Tests/TaeTestWorld.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	// A collection declaring exactly the four names the palette writes. Parameters must exist before
	// a value can be set, so a runtime-built collection has to populate VectorParameters itself.
	UMaterialParameterCollection* MakeArcaneCollection(const bool bIncludeGrowthFront = true)
	{
		UMaterialParameterCollection* Collection = NewObject<UMaterialParameterCollection>();

		auto AddParam = [Collection](const FName& Name)
		{
			FCollectionVectorParameter Param;
			Param.ParameterName = Name;
			Param.DefaultValue = FLinearColor::Black;
			Collection->VectorParameters.Add(Param);
		};

		AddParam(TaeArcaneParams::SpectralEdge);
		AddParam(TaeArcaneParams::CubeTint);
		AddParam(TaeArcaneParams::GroveBloom);
		if (bIncludeGrowthFront)
		{
			AddParam(TaeArcaneParams::GrowthFront);
		}

		return Collection;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTaeArcanePaletteTest,
	"ThroughArcaneEyes.Core.ArcanePalette",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTaeArcanePaletteTest::RunTest(const FString& Parameters)
{
	Tae::Test::FScopedTestWorld Scope;

	UTaeArcanePalette* Palette = NewObject<UTaeArcanePalette>();
	Palette->SpectralEdge = FLinearColor(0.1f, 0.2f, 0.3f, 1.f);
	Palette->CubeTint = FLinearColor(0.4f, 0.5f, 0.6f, 1.f);
	Palette->GroveBloom = FLinearColor(0.7f, 0.8f, 0.9f, 1.f);
	Palette->GrowthFront = FLinearColor(1.f, 0.9f, 0.8f, 1.f);

	UMaterialParameterCollection* Collection = MakeArcaneCollection();

	TestEqual(TEXT("every colour is written"),
		UTaeArcaneSubsystem::ApplyPaletteToCollection(Scope.World, Palette, Collection), 4);

	// The values actually land, not merely report success
	const UMaterialParameterCollectionInstance* Instance = Scope.World->GetParameterCollectionInstance(Collection);
	TestNotNull(TEXT("the world has an instance for the collection"), Instance);

	FLinearColor Read = FLinearColor::Black;
	TestTrue(TEXT("spectral edge reads back"), Instance->GetVectorParameterValue(TaeArcaneParams::SpectralEdge, Read));
	TestEqual(TEXT("spectral edge round-trips"), Read, Palette->SpectralEdge);

	TestTrue(TEXT("grove bloom reads back"), Instance->GetVectorParameterValue(TaeArcaneParams::GroveBloom, Read));
	TestEqual(TEXT("grove bloom round-trips"), Read, Palette->GroveBloom);

	// A collection missing a parameter is the silent-failure case the count exists to catch
	UMaterialParameterCollection* Incomplete = MakeArcaneCollection(false);
	TestEqual(TEXT("a missing parameter is reported, not swallowed"),
		UTaeArcaneSubsystem::ApplyPaletteToCollection(Scope.World, Palette, Incomplete), 3);

	// Null inputs write nothing rather than zeroing the collection
	TestEqual(TEXT("a null palette writes nothing"),
		UTaeArcaneSubsystem::ApplyPaletteToCollection(Scope.World, nullptr, Collection), 0);
	TestEqual(TEXT("a null collection writes nothing"),
		UTaeArcaneSubsystem::ApplyPaletteToCollection(Scope.World, Palette, nullptr), 0);
	TestEqual(TEXT("a null world writes nothing"),
		UTaeArcaneSubsystem::ApplyPaletteToCollection(nullptr, Palette, Collection), 0);

	// The null-palette call must not have clobbered what the good call wrote
	TestTrue(TEXT("spectral edge survives a null write"), Instance->GetVectorParameterValue(TaeArcaneParams::SpectralEdge, Read));
	TestEqual(TEXT("spectral edge is unchanged"), Read, Palette->SpectralEdge);

	return true;
}

#endif
```

- [ ] **Step 2: Run the test to verify it fails**

Run the build command.
Expected: FAIL — `ApplyPaletteToCollection` is not a member of `UTaeArcaneSubsystem`.

- [ ] **Step 3: Implement the write**

In `Public/Core/TaeArcaneSubsystem.h`, add these forward declarations at the top beside the existing ones:

```cpp
class UTaeArcanePalette;
class UMaterialParameterCollection;
```

and this in the `public` section, below `StepBlendAlpha`:

```cpp
	// Copies every palette colour into the collection. Returns how many parameters were written —
	// SetVectorParameterValue returns false for a name the collection does not declare, so a count
	// below the palette's size means the asset and TaeArcaneParams have drifted apart.
	// Static and instance-free so it can be tested against a runtime-built collection.
	static int32 ApplyPaletteToCollection(UWorld* World, const UTaeArcanePalette* Palette, UMaterialParameterCollection* Collection);
```

In `Private/Core/TaeArcaneSubsystem.cpp`, add the includes:

```cpp
#include "Core/TaeArcanePalette.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
```

and the implementation:

```cpp
int32 UTaeArcaneSubsystem::ApplyPaletteToCollection(UWorld* World, const UTaeArcanePalette* Palette, UMaterialParameterCollection* Collection)
{
	if (!World || !Palette || !Collection)
	{
		return 0;
	}

	UMaterialParameterCollectionInstance* Instance = World->GetParameterCollectionInstance(Collection);
	if (!Instance)
	{
		return 0;
	}

	int32 Written = 0;
	Written += Instance->SetVectorParameterValue(TaeArcaneParams::SpectralEdge, Palette->SpectralEdge) ? 1 : 0;
	Written += Instance->SetVectorParameterValue(TaeArcaneParams::CubeTint, Palette->CubeTint) ? 1 : 0;
	Written += Instance->SetVectorParameterValue(TaeArcaneParams::GroveBloom, Palette->GroveBloom) ? 1 : 0;
	Written += Instance->SetVectorParameterValue(TaeArcaneParams::GrowthFront, Palette->GrowthFront) ? 1 : 0;
	return Written;
}
```

- [ ] **Step 4: Run the test to verify it passes**

Run the build command, then the test command with `ThroughArcaneEyes.Core.ArcanePalette`.
Expected: `Test Completed. Result={Success}` for that test, read out of `Saved/Logs/ThroughArcaneEyes.log`.

- [ ] **Step 5: Give the game instance the two assets**

In `Public/Core/TaeGameInstance.h`, add to the forward declarations:

```cpp
class UTaeArcanePalette;
class UMaterialParameterCollection;
```

add to the `public` section beside the music getters:

```cpp
	UTaeArcanePalette* GetArcanePalette() const { return ArcanePalette; }
	UMaterialParameterCollection* GetArcaneCollection() const { return ArcaneCollection; }
```

and to the `protected` section beside the music properties:

```cpp
	// Assign DA_ArcanePalette in BP_TaeGameInstance — the subsystem is a UWorldSubsystem and has no
	// details panel of its own, same reason the music assets live here
	UPROPERTY(EditDefaultsOnly, Category = "Tae|Arcane")
	TObjectPtr<UTaeArcanePalette> ArcanePalette;

	// Assign MPC_Arcane in BP_TaeGameInstance
	UPROPERTY(EditDefaultsOnly, Category = "Tae|Arcane")
	TObjectPtr<UMaterialParameterCollection> ArcaneCollection;
```

No `.cpp` change is needed — both are inline getters and BP-assigned properties.

- [ ] **Step 6: Call it at world start**

In `Private/Core/TaeArcaneSubsystem.cpp`, at the end of `OnWorldBeginPlay`, after the existing post-process volume lookup:

```cpp
	if (const UTaeGameInstance* GI = InWorld.GetGameInstance<UTaeGameInstance>())
	{
		const int32 Written = ApplyPaletteToCollection(&InWorld, GI->GetArcanePalette(), GI->GetArcaneCollection());
		if (Written == 0)
		{
			UE_LOG(LogTae, Warning, TEXT("[Arcane] No palette colours written — assign DA_ArcanePalette and MPC_Arcane in BP_TaeGameInstance"));
		}
	}
```

`Core/TaeGameInstance.h` and `ThroughArcaneEyes.h` are already included in this file; confirm before adding duplicates.

- [ ] **Step 7: Build and run the full suite**

Run the build command, then the test command.
Expected: `Result: Succeeded`, `Found 10 automation tests`, ten `Result={Success}` lines, no `Result={Fail`.

- [ ] **Step 8: Commit**

```powershell
git add Source/ThroughArcaneEyes/Public/Core/TaeArcaneSubsystem.h Source/ThroughArcaneEyes/Private/Core/TaeArcaneSubsystem.cpp Source/ThroughArcaneEyes/Public/Core/TaeGameInstance.h Source/ThroughArcaneEyes/Private/Tests/TaeArcanePaletteTest.cpp
git commit -m "[Core][+] publish the arcane palette to a material parameter collection"
```

---

## Task 4: The grove blooms

The grove currently renders nothing at all, which is why M2's recovery beat happens on invisible ground. This is the highest visual payoff in the plan.

**Files:**
- Modify: `Source/ThroughArcaneEyes/Public/World/TaeGroveComponent.h`
- Modify: `Source/ThroughArcaneEyes/Private/World/TaeGroveComponent.cpp`

**Interfaces:**
- Consumes: nothing from earlier tasks; uses the existing `GetScaledBoxExtent()`, `GetRegenPerSecond()`, and `ActiveRegen`.
- Produces: the `TaeGroveParams` namespace with `Extent`, `RegenPerSecond`, `IsOccupied`; a `BloomSystem` property assigned in `BP_Grove`.

- [ ] **Step 1: Declare the parameter names and the system**

In `Public/World/TaeGroveComponent.h`, add below the existing forward declarations:

```cpp
class UNiagaraComponent;
class UNiagaraSystem;

// User parameter names on NS_GroveBloom. SetVariableFloat and friends fail silently on an unknown
// name, so these constants are the single spelling both C++ and the Niagara asset match against.
namespace TaeGroveParams
{
	inline const FName Extent(TEXT("Extent"));
	inline const FName RegenPerSecond(TEXT("RegenPerSecond"));
	inline const FName IsOccupied(TEXT("IsOccupied"));
}
```

and in the `private` section, beside `RegenCurve`:

```cpp
	// Bloom VFX for this patch of living land. Assign NS_GroveBloom in BP_Grove.
	UPROPERTY(EditAnywhere, Category = "Tae|Grove")
	TObjectPtr<UNiagaraSystem> BloomSystem;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> BloomComponent;
```

- [ ] **Step 2: Spawn it and push the static parameters**

In `Private/World/TaeGroveComponent.cpp`, add the includes:

```cpp
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
```

At the end of `BeginPlay`, after the two overlap delegates are bound:

```cpp
	if (BloomSystem)
	{
		// SpawnSystemAttached is UnsafeDuringActorConstruction, so this cannot move to the constructor
		BloomComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			BloomSystem, this, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset, /*bAutoDestroy=*/false, /*bAutoActivate=*/true);
	}

	if (BloomComponent)
	{
		// The system sizes and scales itself to the grove it is actually on, so resizing the box in
		// BP_Grove needs no second authored value
		BloomComponent->SetVariableVec3(TaeGroveParams::Extent, GetScaledBoxExtent());
		BloomComponent->SetVariableFloat(TaeGroveParams::RegenPerSecond, GetRegenPerSecond());
		BloomComponent->SetVariableBool(TaeGroveParams::IsOccupied, false);
	}
```

- [ ] **Step 3: React to occupancy**

Still in `Private/World/TaeGroveComponent.cpp`, add this helper above `HandleBeginOverlap`:

```cpp
void UTaeGroveComponent::RefreshOccupancy()
{
	if (BloomComponent)
	{
		BloomComponent->SetVariableBool(TaeGroveParams::IsOccupied, ActiveRegen.Num() > 0);
	}
}
```

Call `RefreshOccupancy();` as the last line of `HandleBeginOverlap` and as the last line of `HandleEndOverlap`. Both already maintain `ActiveRegen`, so occupancy rides the callbacks that exist — no tick is added.

Declare it in the header's `private` section:

```cpp
	// Pushes ActiveRegen occupancy to the bloom system. Called from both overlap handlers.
	void RefreshOccupancy();
```

- [ ] **Step 4: Warn when the system is unassigned**

In `IsDataValid`, after the existing `RegenCurve` block:

```cpp
	if (!BloomSystem)
	{
		Context.AddWarning(NSLOCTEXT("TaeValidation", "NoBloomSystem",
			"TaeGroveComponent: BloomSystem is not assigned — this grove is invisible."));
	}
```

A warning rather than an error: a grove with no VFX still regenerates mana, unlike a grove with no curve. Do not change `Result` here — a warning must not make the asset invalid.

- [ ] **Step 5: Build and run the full suite**

Run the build command, then the test command.
Expected: `Result: Succeeded`, `Found 10 automation tests`, all `Result={Success}`. No new test — the parameter values are trivial pass-throughs of methods already covered by `World.GroveRegen`, and the VFX itself is checked visually at the gate.

- [ ] **Step 6: Commit**

```powershell
git add Source/ThroughArcaneEyes/Public/World/TaeGroveComponent.h Source/ThroughArcaneEyes/Private/World/TaeGroveComponent.cpp
git commit -m "[World][+] bloom the grove with a niagara system"
```

---

## Task 5: The growth front

A tip that throws motes as the root advances, so the core verb reads as something happening. The distance arithmetic is a pure static and gets a test; the rest is plumbing.

**Files:**
- Modify: `Source/ThroughArcaneEyes/Public/World/TaeRootPath.h`
- Modify: `Source/ThroughArcaneEyes/Private/World/TaeRootPath.cpp:78-117`
- Create: `Source/ThroughArcaneEyes/Private/Tests/TaeGrowthFrontTest.cpp`
- Modify: `Source/ThroughArcaneEyes/Public/GAS/GA_GrowRoot.h`
- Modify: `Source/ThroughArcaneEyes/Private/GAS/GA_GrowRoot.cpp:98-99, 147-158`

**Interfaces:**
- Consumes: nothing from earlier tasks.
- Produces: `static float ATaeRootPath::GrowthFrontDistance(float GrowthAlpha, float SplineLength)`; `void ATaeRootPath::SetGrowing(bool)`; the `TaeRootParams` namespace with `GrowthAlpha` and `IsGrowing`.

- [ ] **Step 1: Write the failing test**

Create `Source/ThroughArcaneEyes/Private/Tests/TaeGrowthFrontTest.cpp`:

```cpp
// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "Misc/AutomationTest.h"
#include "World/TaeRootPath.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTaeGrowthFrontTest,
	"ThroughArcaneEyes.World.GrowthFront",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTaeGrowthFrontTest::RunTest(const FString& Parameters)
{
	// The front sits where growth has reached
	TestEqual(TEXT("ungrown sits at the start"),
		ATaeRootPath::GrowthFrontDistance(0.f, 1000.f), 0.f);
	TestEqual(TEXT("half grown sits halfway"),
		ATaeRootPath::GrowthFrontDistance(0.5f, 1000.f), 500.f);
	TestEqual(TEXT("fully grown sits at the end"),
		ATaeRootPath::GrowthFrontDistance(1.f, 1000.f), 1000.f);

	// Alpha is clamped rather than running the front off either end of the spline
	TestEqual(TEXT("alpha above one clamps to the end"),
		ATaeRootPath::GrowthFrontDistance(2.f, 1000.f), 1000.f);
	TestEqual(TEXT("negative alpha clamps to the start"),
		ATaeRootPath::GrowthFrontDistance(-1.f, 1000.f), 0.f);

	// A degenerate spline cannot produce a negative distance
	TestEqual(TEXT("zero length yields zero"),
		ATaeRootPath::GrowthFrontDistance(0.5f, 0.f), 0.f);
	TestEqual(TEXT("negative length yields zero"),
		ATaeRootPath::GrowthFrontDistance(0.5f, -100.f), 0.f);

	return true;
}

#endif
```

- [ ] **Step 2: Run the test to verify it fails**

Run the build command.
Expected: FAIL — `GrowthFrontDistance` is not a member of `ATaeRootPath`.

- [ ] **Step 3: Implement the helper and the front component**

In `Public/World/TaeRootPath.h`, add to the forward declarations:

```cpp
class UNiagaraComponent;
class UNiagaraSystem;
```

add below the existing forward declarations:

```cpp
// User parameter names on NS_GrowthFront. SetVariableFloat and friends fail silently on an unknown
// name, so these constants are the single spelling both C++ and the Niagara asset match against.
namespace TaeRootParams
{
	inline const FName GrowthAlpha(TEXT("GrowthAlpha"));
	inline const FName IsGrowing(TEXT("IsGrowing"));
}
```

add to the `public` section, below `GetConnectionState`:

```cpp
	// Distance along the spline where the growing tip sits. Alpha is clamped and a degenerate spline
	// yields zero, so the front never runs off either end. Static and spline-free so it can be tested
	// directly.
	static float GrowthFrontDistance(float InGrowthAlpha, float SplineLength);

	// Called by UGA_GrowRoot when a channel starts and ends. The ability already knows; making the
	// path work it out would mean giving it a tick it does not otherwise need.
	void SetGrowing(bool bNewGrowing);
```

and to the `private` section, beside `PathMaterial`:

```cpp
	// VFX at the growing tip. Assign NS_GrowthFront in BP_RootPath.
	UPROPERTY(EditAnywhere, Category = "RootPath")
	TObjectPtr<UNiagaraSystem> GrowthFrontSystem;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> GrowthFrontComponent;

	// Moves the front component to the tip and pushes GrowthAlpha
	void RefreshGrowthFront();
```

In `Private/World/TaeRootPath.cpp`, add the includes:

```cpp
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
```

Add the pure helper:

```cpp
float ATaeRootPath::GrowthFrontDistance(const float InGrowthAlpha, const float SplineLength)
{
	return FMath::Clamp(InGrowthAlpha, 0.f, 1.f) * FMath::Max(SplineLength, 0.f);
}
```

- [ ] **Step 4: Run the test to verify it passes**

Run the build command, then the test command with `ThroughArcaneEyes.World.GrowthFront`.
Expected: `Test Completed. Result={Success}` for that test.

- [ ] **Step 5: Spawn and drive the front**

In `Private/World/TaeRootPath.cpp`, in `BeginPlay` after the existing `RefreshSegments();` call at `:94`:

```cpp
	if (GrowthFrontSystem)
	{
		// SpawnSystemAttached is UnsafeDuringActorConstruction, so this cannot move to the constructor
		GrowthFrontComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			GrowthFrontSystem, Spline, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset, /*bAutoDestroy=*/false, /*bAutoActivate=*/false);
	}

	RefreshGrowthFront();
```

The system spawns deactivated — an idle path has no growing tip. `SetGrowing` turns it on.

Add the two new methods:

```cpp
void ATaeRootPath::RefreshGrowthFront()
{
	if (!GrowthFrontComponent || !Spline)
	{
		return;
	}

	const float Distance = GrowthFrontDistance(GrowthAlpha, Spline->GetSplineLength());
	GrowthFrontComponent->SetWorldLocation(
		Spline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World));
	GrowthFrontComponent->SetVariableFloat(TaeRootParams::GrowthAlpha, GrowthAlpha);
}

void ATaeRootPath::SetGrowing(const bool bNewGrowing)
{
	if (!GrowthFrontComponent)
	{
		return;
	}

	// A finished connection has no growing tip, whatever the ability thinks
	const bool bActuallyGrowing = bNewGrowing && ConnectionState != ETaeConnectionState::Restored;

	GrowthFrontComponent->SetVariableBool(TaeRootParams::IsGrowing, bActuallyGrowing);
	if (bActuallyGrowing)
	{
		GrowthFrontComponent->Activate();
	}
	else
	{
		GrowthFrontComponent->Deactivate();
	}
}
```

In `AdvanceGrowth`, immediately after the existing `RefreshSegments();` call at `:111`:

```cpp
	RefreshGrowthFront();
```

`AdvanceGrowth` early-returns when the alpha has not changed, so an idle path costs nothing.

- [ ] **Step 6: Have the ability drive the flag**

In `Private/GAS/GA_GrowRoot.cpp`, immediately after the `SetTimer` call at `:98-99`:

```cpp
	ActivePath->SetGrowing(true);
```

In `EndAbility`, immediately before `ActivePath = nullptr;` at `:158`:

```cpp
	if (ActivePath)
	{
		ActivePath->SetGrowing(false);
	}
```

`World/TaeRootPath.h` is already included in this file; confirm before adding a duplicate. No header change is needed for `GA_GrowRoot.h` — `SetGrowing` is called through the existing `ActivePath` pointer.

- [ ] **Step 7: Build and run the full suite**

Run the build command, then the test command.
Expected: `Result: Succeeded`, `Found 11 automation tests`, eleven `Result={Success}` lines, no `Result={Fail`. `World.GrowthStep` and both `GAS.ManaDrain*` tests must still pass — Task 5 touches the growth path they exercise.

- [ ] **Step 8: Commit**

```powershell
git add Source/ThroughArcaneEyes/Public/World/TaeRootPath.h Source/ThroughArcaneEyes/Private/World/TaeRootPath.cpp Source/ThroughArcaneEyes/Private/Tests/TaeGrowthFrontTest.cpp Source/ThroughArcaneEyes/Private/GAS/GA_GrowRoot.cpp
git commit -m "[World][+] throw vfx from the growing root tip"
```

---

## Task 6: Editor assets and the handoff

Everything above compiles and does nothing visible until the assets exist. This task scripts what can be scripted and documents what cannot.

**Files:**
- Create: `Tools/Python/tae_vfx_assets.py`
- Create: `docs/issues/2026-08-15-arcane-presentation-handoff.md`

**Interfaces:**
- Consumes: `TaeArcaneParams` (Task 2) — the MPC's parameter names must match it exactly.
- Produces: `Content/Locations/Materials/MPC_Arcane`, `Content/Core/DA_ArcanePalette`.

**What Python can and cannot do here — do not rediscover this:**

- **Creating a `UMaterialParameterCollection` and populating `VectorParameters` works.** `MaterialParameterCollectionFactoryNew` plus `set_editor_property` on the struct array.
- **Creating a `UTaeArcanePalette` instance works** — `DataAssetFactory` with the class set.
- **Niagara systems cannot be usefully scripted.** Emitter graphs are not exposed; a scripted `NS_` asset would be an empty shell, not a starting point. `NS_GroveBloom` and `NS_GrowthFront` are authored by hand.
- **`EditorActorSubsystem.spawn_actor_from_object` crashes the `-run=pythonscript` commandlet** (access violation in `EditorFramework.dll` after `load_level`). Nothing here needs actor placement — do not add any.

- [ ] **Step 1: Write the asset script**

Create `Tools/Python/tae_vfx_assets.py`:

```python
# Copyright (c) 2026 Helen Allien Poe. Source available - see LICENSE.
"""Creates the arcane presentation assets: MPC_Arcane and DA_ArcanePalette.

Run headless:
  UnrealEditor-Cmd.exe <project>.uproject -run=pythonscript -script="<this file>" -unattended -nopause -nosplash

The four MPC parameter names are load-bearing: they must match TaeArcaneParams in
Public/Core/TaeArcanePalette.h character for character. SetVectorParameterValue returns false for
a name the collection does not declare, so a mismatch surfaces as ApplyPaletteToCollection
returning fewer than 4 and a "[Arcane] No palette colours written" warning in the log.

Idempotent: safe to run more than once. Every asset is checked for existence before creation.

Niagara systems are deliberately NOT here. Emitter graphs are not exposed to Python, so a scripted
NS_ asset would be an empty shell rather than a starting point. NS_GroveBloom and NS_GrowthFront
are authored by hand - see docs/issues/2026-08-15-arcane-presentation-handoff.md.

Unlike UCurveFloat.FloatCurve (see the CSV workaround in tae_m2_assets.py),
UMaterialParameterCollection.VectorParameters is UPROPERTY(EditAnywhere) and therefore reachable
from Python directly. No import workaround needed here.
"""

import unreal

MATERIALS_PATH = "/Game/Locations/Materials"
CORE_PATH = "/Game/Core"

# Must match TaeArcaneParams in Public/Core/TaeArcanePalette.h, and the defaults should match the
# UPROPERTY initialisers on UTaeArcanePalette so an unassigned palette still looks deliberate.
ARCANE_PARAMS = [
    ("SpectralEdge", unreal.LinearColor(0.2, 0.8, 1.0, 1.0)),
    ("CubeTint", unreal.LinearColor(0.1, 0.4, 0.6, 1.0)),
    ("GroveBloom", unreal.LinearColor(0.4, 0.9, 0.35, 1.0)),
    ("GrowthFront", unreal.LinearColor(0.6, 1.0, 0.5, 1.0)),
]

assets = unreal.AssetToolsHelpers.get_asset_tools()


def make_parameter_collection():
    full = "{}/{}".format(MATERIALS_PATH, "MPC_Arcane")
    if unreal.EditorAssetLibrary.does_asset_exist(full):
        unreal.log("exists, skipping: {}".format(full))
        return unreal.EditorAssetLibrary.load_asset(full)

    factory = unreal.MaterialParameterCollectionFactoryNew()
    collection = assets.create_asset(
        "MPC_Arcane", MATERIALS_PATH, unreal.MaterialParameterCollection, factory)

    params = []
    for name, default in ARCANE_PARAMS:
        param = unreal.CollectionVectorParameter()
        param.set_editor_property("parameter_name", name)
        param.set_editor_property("default_value", default)
        params.append(param)

    collection.set_editor_property("vector_parameters", params)
    unreal.EditorAssetLibrary.save_asset(full)
    unreal.log("created: {} with {} vector parameters".format(full, len(params)))
    return collection


def make_palette():
    full = "{}/{}".format(CORE_PATH, "DA_ArcanePalette")
    if unreal.EditorAssetLibrary.does_asset_exist(full):
        unreal.log("exists, skipping: {}".format(full))
        return unreal.EditorAssetLibrary.load_asset(full)

    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", unreal.TaeArcanePalette)
    palette = assets.create_asset(
        "DA_ArcanePalette", CORE_PATH, unreal.TaeArcanePalette, factory)

    # Left at the C++ defaults deliberately - this asset is the artist's tuning surface
    unreal.EditorAssetLibrary.save_asset(full)
    unreal.log("created: {}".format(full))
    return palette


def main():
    make_parameter_collection()
    make_palette()
    unreal.log("arcane presentation assets done")


main()
```

**One uncertainty, with a cheap fallback.** `FCollectionVectorParameter` is a bare `USTRUCT()` without `BlueprintType` (`MaterialParameterCollection.h:58-59`), so `unreal.CollectionVectorParameter` may not be exposed to Python. If the script fails on that line, do **not** spend time fighting it — delete `make_parameter_collection`, create `MPC_Arcane` by hand in the Content Browser, and add the four vector parameters in its details panel. That is a thirty-second job, and the names in `ARCANE_PARAMS` are the list to type. Keep `make_palette`, which uses only `EditAnywhere` properties and is not at risk.

- [ ] **Step 2: Run the script**

```powershell
& 'D:\EpicGames\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' "D:\PetProjects\ThroughArcaneEyes\ThroughArcaneEyes.uproject" -run=pythonscript -script="D:\PetProjects\ThroughArcaneEyes\Tools\Python\tae_vfx_assets.py" -unattended -nopause -nosplash
```

Expected: both assets exist under `Content/Locations/Materials/MPC_Arcane.uasset` and `Content/Core/DA_ArcanePalette.uasset`.

- [ ] **Step 3: Verify the wiring headlessly**

Run the headless game command from Global Constraints.
Expected: **no** `[Arcane] No palette colours written` warning once the assets are assigned in `BP_TaeGameInstance`. Before assignment the warning is expected and correct — it is what tells you the assignment step is still owed.

- [ ] **Step 4: Write the handoff doc**

Create `docs/issues/2026-08-15-arcane-presentation-handoff.md`.

Note that `docs/issues/` is gitignored — that directory is the working area between the user and the agent. This handoff lives there deliberately, the same as the M2 editor handoff's successor. If it turns out to hold decisions worth keeping, promote them into the spec rather than un-ignoring the directory.

It must cover, in order:

1. **Assign the assets.** `DA_ArcanePalette` and `MPC_Arcane` into `BP_TaeGameInstance` (Class Defaults → Tae|Arcane).
2. **Author `NS_GroveBloom`**, assign it to the `UTaeGroveComponent` in `BP_Grove`. User parameters, matching `TaeGroveParams` exactly: `Extent` (Vector), `RegenPerSecond` (float), `IsOccupied` (bool). Colour samples `MPC_Arcane.GroveBloom`. The grove box is `(700, 700, 200)` — the system receives that as `Extent` and should fill it.
3. **Author `NS_GrowthFront`**, assign it to `BP_RootPath`. User parameters, matching `TaeRootParams` exactly: `GrowthAlpha` (float), `IsGrowing` (bool). Colour samples `MPC_Arcane.GrowthFront`. It spawns deactivated and is activated by `SetGrowing`.
4. **Re-point the materials.** `M_SpectralEdge` samples `MPC_Arcane.SpectralEdge`; `M_GridCube_Arcane` samples `MPC_Arcane.CubeTint`. Capture a PIE screenshot before and after — spec §9 flags this as the one step that could regress the existing look.
5. **Record the gate clip** — this is M2's deferred clip, per spec §7. Include the staging note: with `0.35` / `12` defaults a connection costs ~46 mana against a 100 bar, so survey for ~10 s first to burn the bar down, or running dry mid-channel will never happen.
6. **Then tuning**, which spec §7 puts explicitly after this pass, not inside it.

- [ ] **Step 5: Commit**

`docs/issues/` is gitignored, so only the script is committed.

```powershell
git add Tools/Python/tae_vfx_assets.py
git commit -m "[Config][+] script the arcane palette and parameter collection"
```

---

## Done when

- The suite reports `Found 11 automation tests`, all `Result={Success}`.
- A headless game run logs no `[Arcane] No palette colours written` warning.
- The grove is visible in PIE and reacts when the player stands in it.
- The root's growing tip throws VFX while channelling and stops when the channel ends or the connection completes.
- `M_SpectralEdge` and `M_GridCube_Arcane` draw their colours from `MPC_Arcane` rather than private copies.
- M2's deferred gate clip is recorded, which closes M2's last outstanding artifact.

Balance tuning is **not** part of this plan — spec §7 sequences it after, as an editor-only pass.
