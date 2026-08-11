# M2 — Mana Has Teeth — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make Arcane Vision cost something — it drains mana while active, growing drains faster, running dry ejects the player to Forest and locks re-entry until they recover, and recovery happens only while standing on living land.

**Architecture:** Three flows (vision drain, growth drain, grove regen) are all Infinite Gameplay Effects with a 0.1s period and a SetByCaller magnitude, applied and removed by whoever owns the flow. The effects are C++ classes carrying no numbers; every rate is an `EditDefaultsOnly` float on the spender. Exhaustion is a state machine on `UTaeManaAttributeSet` that toggles `Arcane.Exhausted`, which `UGA_SpectralShift` blocks and cancels on — `UGA_GrowRoot` needs no exhaustion logic because it already cancels when `Arcane.Vision` drops.

**Tech Stack:** UE 5.8 C++, GAS (`GameplayAbilities`), MVVM (`ModelViewViewModel`), UE Automation Tests, `PythonScriptPlugin` for editor asset work.

**Source spec:** [`docs/superpowers/specs/2026-08-11-m2-mana-economy-design.md`](../specs/2026-08-11-m2-mana-economy-design.md)

## Global Constraints

- Engine: UE 5.8 at `D:\EpicGames\UE_5.8`. Project: `D:\PetProjects\ThroughArcaneEyes`.
- Single runtime module `ThroughArcaneEyes`. No new Build.cs modules; M2 adds no new dependencies.
- Every file starts with: `// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.`
- Naming: `A`+`Tae` actors, `U`+`Tae` objects/components, `F` structs, `E` enums. API macro `THROUGHARCANEEYES_API`.
- **All `UPROPERTY` members use `TObjectPtr<T>`, never raw `T*`.** Forward-declare in headers, include in `.cpp`.
- Logging: `LogTae` only, never `LogTemp`. `Warning` for null-guards only, no flow logging.
- Prefer `IsDataValid` under `#if WITH_EDITOR` over runtime null-guards for BP-assigned properties. Include `Misc/DataValidation.h` in the `.cpp`.
- Gameplay tags declared in `Public/GAS/TaeGASTypes.h`, defined in `Private/GAS/TaeGASTypes.cpp` via `UE_DEFINE_GAMEPLAY_TAG`. **No `FName` tag strings in calling code.**
- All input handled in `ATaePlayerController`, never in `ATaeCharacter`.
- Source mirrors headers: `Public/<Domain>/TaeX.h` → `Private/<Domain>/TaeX.cpp`.
- All economy rates are `EditDefaultsOnly` on the class that spends them — never baked into an effect.
- Commit format: `[TAG][sigil] short description`. Sigils `[+]` add, `[-]` remove, `[*]` fix/tweak. Tags used here: `[GAS]`, `[World]`, `[UI]`, `[Character]`, `[Config]`, `[Docs]`.

**Build command** (run after every code change):

```powershell
& 'D:\EpicGames\UE_5.8\Engine\Build\BatchFiles\Build.bat' ThroughArcaneEyesEditor Win64 Development -Project="D:\PetProjects\ThroughArcaneEyes\ThroughArcaneEyes.uproject" -WaitMutex
```
Expected on success: `Result: Succeeded`.

**Test command** (verified against the baseline run on 2026-08-11):

```powershell
& 'D:\EpicGames\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' "D:\PetProjects\ThroughArcaneEyes\ThroughArcaneEyes.uproject" -ExecCmds="Automation RunTests ThroughArcaneEyes; Quit" -unattended -nopause -nullrhi -nosplash -log
Select-String -Path "D:\PetProjects\ThroughArcaneEyes\Saved\Logs\ThroughArcaneEyes.log" -Pattern "Found \d+ automation tests|Test Completed" | ForEach-Object { $_.Line }
```

**Two things that will waste your time if you skip them:**

1. **Results do not go to stdout.** The command's console output contains only UBT SDK-validation noise. Every test result lands in `Saved/Logs/ThroughArcaneEyes.log`, which is why the `Select-String` line above is part of the command, not optional.
2. **The engine prints `Result={Success}`, not `Result={Passed}`.** A failing test prints `Result={Fail}`.

Expected on success: one `Test Completed. Result={Success}` line per test, a `Found N automation tests` line where N matches the number you expect, and no `Result={Fail`.

Baseline before M2 (branch `feature/m2-mana-economy` at `527129e`): 3 tests — `Core.BlendAlpha`, `Harness.Sanity`, `World.GrowthStep` — all `Success`.

Single test: replace `ThroughArcaneEyes` in `RunTests` with the full test name, e.g. `ThroughArcaneEyes.GAS.ManaExhaustion`.

## Engine API notes (verified against 5.8 source — do not substitute from memory)

- `EGameplayModOp::Additive` is a hidden backwards-compat alias. **Use `EGameplayModOp::AddBase`** (`GameplayEffectTypes.h:121`).
- `UGameplayEffect::OngoingTagRequirements` is **deprecated** (`GameplayEffect.h:2360`). Use `UTargetTagRequirementsGameplayEffectComponent`.
- **Do not create that component with `FindOrAddComponent<T>()` in a constructor** — it calls `NewObject` with `NAME_None` (`GameplayEffect.h:2500`), which is fatal inside a `UObject` constructor and crashes the editor at startup. Every engine call site is inside a `ConvertXComponent()` helper run from `PostCDOCompiled` (`GameplayEffect.cpp:529-825`). For a native GE, use `CreateDefaultSubobject` and add the result to the `protected` `GEComponents` array — `UGameplayEffect::PostInitProperties` explicitly expects this, ensuring with *"should be added to GEComponents during the constructor or in PostInitProperties"* (`GameplayEffect.cpp:236`). *(Found by Task 3's implementer hitting the crash; corrected 2026-08-11.)*
- `UGameplayEffect::Period` is an `FScalableFloat` (`GameplayEffect.h:2268`).
- A periodic modifier applies its magnitude **once per period, not per second**. Magnitude must be `Rate * PeriodSeconds`.
- `FGameplayEffectModifierMagnitude` has an `FSetByCallerFloat` constructor (`GameplayEffect.h:304`); `FSetByCallerFloat::DataTag` is the tag-keyed form.
- Gameplay Cue notifies are discovered by an **asset registry scan** (`GameplayCueManager.cpp:830-896`). Pure C++ notify classes are never registered — each cue needs a Blueprint asset whose **name derives the tag** (`GC_Mana_Drain` → `GameplayCue.Mana.Drain`). C++ parents must leave `GameplayCueTag` unset.

---

## File Structure

| File | Responsibility |
|---|---|
| `Public/GAS/TaeGASTypes.h` / `Private/GAS/TaeGASTypes.cpp` | **Modify.** Add the exhaustion tag, three cue tags, and the SetByCaller data tag. |
| `Public/GAS/TaeManaAttributeSet.h` / `.cpp` | **Modify.** `EvaluateExhaustion` pure static, `PostAttributeChange` tag toggling, `RecoveryFraction`. |
| `Private/Tests/TaeManaExhaustionTest.cpp` | **Create.** Automation tests for the hysteresis. |
| `Public/GAS/TaeManaEffects.h` / `Private/GAS/TaeManaEffects.cpp` | **Create.** `UTaeManaEffectBase` + drain and regen subclasses. Tiny and always changed together, so one file pair. |
| `Private/Tests/TaeManaEffectsTest.cpp` | **Create.** Automation tests for per-period magnitude conversion. |
| `Public/GAS/GA_SpectralShift.h` / `.cpp` | **Modify.** Apply/remove the drain; block and cancel on exhaustion. |
| `Public/GAS/GA_GrowRoot.h` / `.cpp` | **Modify.** Replace direct attribute writes with the drain effect. |
| `Public/GAS/TaeCueNotifies.h` / `Private/GAS/TaeCueNotifies.cpp` | **Create.** Three `UGameplayCueNotify_Static` subclasses. |
| `Public/World/TaeGroveComponent.h` / `.cpp` | **Create.** Box footprint, area→rate curve, applies regen on overlap. |
| `Private/Tests/TaeGroveRegenTest.cpp` | **Create.** Automation tests for area→rate. |
| `Public/UI/TaeHudViewModel.h` / `.cpp` | **Modify.** `MaxMana`, `ManaPercent`, `bExhausted`, flow counters, tint. |
| `Private/Tests/TaeManaFlowTest.cpp` | **Create.** Automation tests for flow resolution. |
| `Public/Character/TaeCharacter.h` / `.cpp` | **Modify.** `ExhaustionRecoveryFraction`, pushed into the attribute set. |
| `Public/Character/TaePlayerController.h` / `.cpp` | **Modify.** Bind `MaxMana` and the `Arcane.Exhausted` tag to the ViewModel. |
| `ThroughArcaneEyes.uproject` | **Modify.** Enable `PythonScriptPlugin`. |
| `Tools/Python/tae_m2_assets.py` | **Create.** Creates the cue Blueprints, regen curve, `BP_Grove`, and places it in `WorldNull`. |
| `docs/issues/2026-08-11-m2-hud-handoff.md` | **Create.** The one manual editor step. |

---

## Task 1: Tags and the exhaustion state machine

The hysteresis is the only genuinely tricky logic in M2 — two thresholds that must never drift apart. It ships first, as a pure function with tests, before anything depends on it.

**Files:**
- Modify: `Source/ThroughArcaneEyes/Public/GAS/TaeGASTypes.h:16-17`
- Modify: `Source/ThroughArcaneEyes/Private/GAS/TaeGASTypes.cpp:5-6`
- Modify: `Source/ThroughArcaneEyes/Public/GAS/TaeManaAttributeSet.h`
- Modify: `Source/ThroughArcaneEyes/Private/GAS/TaeManaAttributeSet.cpp`
- Create: `Source/ThroughArcaneEyes/Private/Tests/TaeManaExhaustionTest.cpp`

**Interfaces:**
- Consumes: nothing.
- Produces: `TAG_Arcane_Exhausted`, `TAG_Cue_Mana_Drain`, `TAG_Cue_Mana_Regen`, `TAG_Cue_Arcane_Exhausted`, `TAG_Data_ManaRate`; and `static bool UTaeManaAttributeSet::EvaluateExhaustion(float Mana, float MaxMana, float RecoveryFraction, bool bWasExhausted)`.

- [ ] **Step 1: Add the native tags**

In `Public/GAS/TaeGASTypes.h`, after the existing two declarations:

```cpp
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Arcane_Exhausted)

// Gameplay Cue tags. The matching Blueprint assets MUST be named so these derive from the asset name —
// GC_Mana_Drain -> GameplayCue.Mana.Drain. See the M2 spec §6.
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cue_Mana_Drain)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cue_Mana_Regen)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Cue_Arcane_Exhausted)

// SetByCaller key for the per-period mana delta on UTaeManaEffectBase
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Data_ManaRate)
```

In `Private/GAS/TaeGASTypes.cpp`, after the existing two definitions:

```cpp
UE_DEFINE_GAMEPLAY_TAG(TAG_Arcane_Exhausted, "Arcane.Exhausted")
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Mana_Drain, "GameplayCue.Mana.Drain")
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Mana_Regen, "GameplayCue.Mana.Regen")
UE_DEFINE_GAMEPLAY_TAG(TAG_Cue_Arcane_Exhausted, "GameplayCue.Arcane.Exhausted")
UE_DEFINE_GAMEPLAY_TAG(TAG_Data_ManaRate, "SetByCaller.ManaRate")
```

- [ ] **Step 2: Write the failing test**

Create `Source/ThroughArcaneEyes/Private/Tests/TaeManaExhaustionTest.cpp`:

```cpp
// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "Misc/AutomationTest.h"
#include "GAS/TaeManaAttributeSet.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTaeManaExhaustionTest,
	"ThroughArcaneEyes.GAS.ManaExhaustion",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTaeManaExhaustionTest::RunTest(const FString& Parameters)
{
	// Healthy mana never triggers exhaustion
	TestFalse(TEXT("half full is not exhausted"),
		UTaeManaAttributeSet::EvaluateExhaustion(50.f, 100.f, 0.25f, false));

	// Empty triggers it
	TestTrue(TEXT("empty becomes exhausted"),
		UTaeManaAttributeSet::EvaluateExhaustion(0.f, 100.f, 0.25f, false));

	// Once exhausted, a trickle is not enough — this is the hysteresis
	TestTrue(TEXT("stays exhausted below the floor"),
		UTaeManaAttributeSet::EvaluateExhaustion(10.f, 100.f, 0.25f, true));
	TestTrue(TEXT("stays exhausted just below the floor"),
		UTaeManaAttributeSet::EvaluateExhaustion(24.9f, 100.f, 0.25f, true));

	// Reaching the floor recovers
	TestFalse(TEXT("recovers at the floor"),
		UTaeManaAttributeSet::EvaluateExhaustion(25.f, 100.f, 0.25f, true));

	// A fraction above 1 is clamped rather than making recovery impossible
	TestFalse(TEXT("fraction above one clamps to full"),
		UTaeManaAttributeSet::EvaluateExhaustion(100.f, 100.f, 2.f, true));

	// A zero fraction means any mana at all recovers
	TestFalse(TEXT("zero fraction recovers immediately"),
		UTaeManaAttributeSet::EvaluateExhaustion(0.f, 100.f, 0.f, true));

	return true;
}

#endif
```

- [ ] **Step 3: Run the test to verify it fails**

Run the build command. Expected: FAIL — `EvaluateExhaustion` is not a member of `UTaeManaAttributeSet`.

- [ ] **Step 4: Implement the pure function**

In `Public/GAS/TaeManaAttributeSet.h`, inside the class, above the `UPROPERTY` block:

```cpp
	// Pure exhaustion state machine with hysteresis: exhaustion begins at empty and ends only once mana
	// climbs back to RecoveryFraction of maximum. Static and ASC-free so both thresholds are tested
	// together and cannot drift apart. Returns the new exhausted state.
	static bool EvaluateExhaustion(float Mana, float MaxManaValue, float RecoveryFraction, bool bWasExhausted);
```

In `Private/GAS/TaeManaAttributeSet.cpp`:

```cpp
bool UTaeManaAttributeSet::EvaluateExhaustion(const float Mana, const float MaxManaValue, const float RecoveryFraction, const bool bWasExhausted)
{
	if (!bWasExhausted)
	{
		return Mana <= 0.f;
	}

	const float RecoverAt = FMath::Clamp(RecoveryFraction, 0.f, 1.f) * MaxManaValue;
	return Mana < RecoverAt;
}
```

- [ ] **Step 5: Run the test to verify it passes**

Run the build command, then the test command with `ThroughArcaneEyes.GAS.ManaExhaustion`.
Expected: `Test Completed. Result={Success}` for that test (read it out of `Saved/Logs/ThroughArcaneEyes.log`, not stdout).

- [ ] **Step 6: Commit**

```powershell
git add Source/ThroughArcaneEyes/Public/GAS/TaeGASTypes.h Source/ThroughArcaneEyes/Private/GAS/TaeGASTypes.cpp Source/ThroughArcaneEyes/Public/GAS/TaeManaAttributeSet.h Source/ThroughArcaneEyes/Private/GAS/TaeManaAttributeSet.cpp Source/ThroughArcaneEyes/Private/Tests/TaeManaExhaustionTest.cpp
git commit -m "[GAS][+] add exhaustion state machine and economy tags"
```

---

## Task 2: The attribute set toggles the exhausted tag

Wires the pure function from Task 1 into the live attribute, and gives the recovery floor an editable home.

**Files:**
- Modify: `Source/ThroughArcaneEyes/Public/GAS/TaeManaAttributeSet.h`
- Modify: `Source/ThroughArcaneEyes/Private/GAS/TaeManaAttributeSet.cpp`
- Modify: `Source/ThroughArcaneEyes/Public/Character/TaeCharacter.h:48-58`
- Modify: `Source/ThroughArcaneEyes/Private/Character/TaeCharacter.cpp:33-51`

**Interfaces:**
- Consumes: `UTaeManaAttributeSet::EvaluateExhaustion`, `TAG_Arcane_Exhausted`, `TAG_Cue_Arcane_Exhausted` (Task 1).
- Produces: `void UTaeManaAttributeSet::SetRecoveryFraction(float)`; the `Arcane.Exhausted` loose tag appearing on the character's ASC; the `GameplayCue.Arcane.Exhausted` burst firing on the transition into exhaustion.

- [ ] **Step 1: Add the recovery fraction and the override**

In `Public/GAS/TaeManaAttributeSet.h`, add below `EvaluateExhaustion`:

```cpp
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	// Set from ATaeCharacter::BeginPlay — attribute sets are not editable in the Blueprint details panel,
	// so the character owns the editable value.
	void SetRecoveryFraction(float NewFraction) { RecoveryFraction = NewFraction; }
```

and as a private member at the end of the class:

```cpp
private:
	// Fraction of MaxMana that must be reached before Arcane Vision is available again
	float RecoveryFraction = 0.25f;
```

- [ ] **Step 2: Implement the toggle**

In `Private/GAS/TaeManaAttributeSet.cpp`, add the include and the override:

```cpp
#include "GAS/TaeGASTypes.h"

void UTaeManaAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, const float OldValue, const float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	// MaxMana matters too — it moves the recovery floor
	if (Attribute != GetManaAttribute() && Attribute != GetMaxManaAttribute())
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	const bool bWasExhausted = ASC->HasMatchingGameplayTag(TAG_Arcane_Exhausted);
	const bool bNowExhausted = EvaluateExhaustion(GetMana(), GetMaxMana(), RecoveryFraction, bWasExhausted);
	if (bNowExhausted == bWasExhausted)
	{
		return;
	}

	FGameplayTagContainer ExhaustedTag;
	ExhaustedTag.AddTag(TAG_Arcane_Exhausted);

	if (bNowExhausted)
	{
		ASC->AddLooseGameplayTags(ExhaustedTag);
		ASC->ExecuteGameplayCue(TAG_Cue_Arcane_Exhausted);
	}
	else
	{
		ASC->RemoveLooseGameplayTags(ExhaustedTag);
	}
}
```

- [ ] **Step 3: Give the character the editable fraction**

In `Public/Character/TaeCharacter.h`, in the private section beside the other GAS properties:

```cpp
	// Fraction of MaxMana required to leave exhaustion and re-enter Arcane Vision.
	// Tune in BP_TaeCharacter Class Defaults.
	UPROPERTY(EditDefaultsOnly, Category = "GAS", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ExhaustionRecoveryFraction = 0.25f;
```

In `Private/Character/TaeCharacter.cpp`, in `BeginPlay`, immediately after `Super::BeginPlay();`:

```cpp
	if (ManaAttributeSet)
	{
		ManaAttributeSet->SetRecoveryFraction(ExhaustionRecoveryFraction);
	}
```

- [ ] **Step 4: Build and run the full suite**

Run the build command, then the test command.
Expected: `Result: Succeeded`, and the Task 1 test still `Result={Passed}`.

- [ ] **Step 5: Commit**

```powershell
git add Source/ThroughArcaneEyes/Public/GAS/TaeManaAttributeSet.h Source/ThroughArcaneEyes/Private/GAS/TaeManaAttributeSet.cpp Source/ThroughArcaneEyes/Public/Character/TaeCharacter.h Source/ThroughArcaneEyes/Private/Character/TaeCharacter.cpp
git commit -m "[GAS][+] toggle the exhausted tag from the mana attribute"
```

---

## Task 3: The mana effects

Two Gameplay Effects in C++, carrying no numbers. The per-period conversion is the part that silently ruins tuning if it is wrong, so it is a tested pure function.

**Files:**
- Create: `Source/ThroughArcaneEyes/Public/GAS/TaeManaEffects.h`
- Create: `Source/ThroughArcaneEyes/Private/GAS/TaeManaEffects.cpp`
- Create: `Source/ThroughArcaneEyes/Private/Tests/TaeManaEffectsTest.cpp`

**Interfaces:**
- Consumes: `TAG_Data_ManaRate`, `TAG_Cue_Mana_Drain`, `TAG_Cue_Mana_Regen`, `TAG_Arcane_Vision` (Task 1 and existing).
- Produces: `UTaeManaEffectBase` with `static constexpr float PeriodSeconds` and `static float MagnitudePerPeriod(float RatePerSecond)`; `UTaeManaDrainEffect`; `UTaeManaRegenEffect`.

- [ ] **Step 1: Write the failing test**

Create `Source/ThroughArcaneEyes/Private/Tests/TaeManaEffectsTest.cpp`:

```cpp
// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "Misc/AutomationTest.h"
#include "GAS/TaeManaEffects.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTaeManaEffectsTest,
	"ThroughArcaneEyes.GAS.ManaEffects",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTaeManaEffectsTest::RunTest(const FString& Parameters)
{
	// A periodic modifier applies once per period, so a per-second rate must be scaled down
	TestEqual(TEXT("ten per second over a tenth-second period"),
		UTaeManaEffectBase::MagnitudePerPeriod(10.f), 1.f);

	// Sign is carried through — callers pass negative rates for drains
	TestEqual(TEXT("negative rates stay negative"),
		UTaeManaEffectBase::MagnitudePerPeriod(-12.f), -1.2f);

	TestEqual(TEXT("zero is zero"),
		UTaeManaEffectBase::MagnitudePerPeriod(0.f), 0.f);

	// The period is the contract the magnitude depends on — pin it
	TestEqual(TEXT("period is a tenth of a second"),
		UTaeManaEffectBase::PeriodSeconds, 0.1f);

	return true;
}

#endif
```

- [ ] **Step 2: Run the test to verify it fails**

Run the build command. Expected: FAIL — `GAS/TaeManaEffects.h` does not exist.

- [ ] **Step 3: Write the header**

Create `Source/ThroughArcaneEyes/Public/GAS/TaeManaEffects.h`:

```cpp
// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "TaeManaEffects.generated.h"

// Shared shape for every mana flow: an infinite effect that ticks a SetByCaller delta onto Mana.
// Carries no rates — callers supply the magnitude, so one class serves vision drain, growth drain,
// and grove regen at three different rates.
UCLASS(Abstract)
class THROUGHARCANEEYES_API UTaeManaEffectBase : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UTaeManaEffectBase();

	// How often the effect ticks. Magnitudes are per-period, not per-second.
	static constexpr float PeriodSeconds = 0.1f;

	// Converts a designer-facing per-second rate into the per-period magnitude GAS actually applies.
	// Negative rates drain, positive rates restore.
	static float MagnitudePerPeriod(const float RatePerSecond) { return RatePerSecond * PeriodSeconds; }
};

// Applied by UGA_SpectralShift while Arcane Vision is active, and again by UGA_GrowRoot while
// channelling. Two applications tick independently, so the drains add up.
UCLASS()
class THROUGHARCANEEYES_API UTaeManaDrainEffect : public UTaeManaEffectBase
{
	GENERATED_BODY()

public:
	UTaeManaDrainEffect();
};

// Applied by UTaeGroveComponent while the player stands in a grove. Inhibited — not removed — while
// Arcane Vision is active, so recovery requires dropping back to Forest.
UCLASS()
class THROUGHARCANEEYES_API UTaeManaRegenEffect : public UTaeManaEffectBase
{
	GENERATED_BODY()

public:
	UTaeManaRegenEffect();
};
```

- [ ] **Step 4: Write the implementation**

Create `Source/ThroughArcaneEyes/Private/GAS/TaeManaEffects.cpp`:

```cpp
// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "GAS/TaeManaEffects.h"
#include "GAS/TaeGASTypes.h"
#include "GAS/TaeManaAttributeSet.h"
#include "GameplayEffectComponents/TargetTagRequirementsGameplayEffectComponent.h"

UTaeManaEffectBase::UTaeManaEffectBase()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	Period = FScalableFloat(PeriodSeconds);

	// The first tick lands one period in, so applying and immediately removing costs nothing
	bExecutePeriodicEffectOnApplication = false;

	FSetByCallerFloat RateMagnitude;
	RateMagnitude.DataTag = TAG_Data_ManaRate;

	FGameplayModifierInfo ManaModifier;
	ManaModifier.Attribute = UTaeManaAttributeSet::GetManaAttribute();
	// AddBase, not the hidden backwards-compat Additive alias
	ManaModifier.ModifierOp = EGameplayModOp::AddBase;
	ManaModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(RateMagnitude);
	Modifiers.Add(ManaModifier);
}

UTaeManaDrainEffect::UTaeManaDrainEffect()
{
	FGameplayEffectCue DrainCue;
	DrainCue.GameplayCueTags.AddTag(TAG_Cue_Mana_Drain);
	GameplayCues.Add(DrainCue);
}

UTaeManaRegenEffect::UTaeManaRegenEffect()
{
	FGameplayEffectCue RegenCue;
	RegenCue.GameplayCueTags.AddTag(TAG_Cue_Mana_Regen);
	GameplayCues.Add(RegenCue);

	// Standing in a grove does not refill you mid-survey. Inhibition keeps the effect applied, so
	// leaving Arcane resumes regen without re-entering the volume.
	//
	// UGameplayEffect::OngoingTagRequirements is deprecated in 5.8, so the requirement lives on a
	// component. It must be created with CreateDefaultSubobject rather than FindOrAddComponent:
	// FindOrAddComponent calls NewObject with an empty name, which is fatal inside a constructor.
	// UGameplayEffect::PostInitProperties expects native components to be made exactly this way.
	UTargetTagRequirementsGameplayEffectComponent* TagRequirements =
		CreateDefaultSubobject<UTargetTagRequirementsGameplayEffectComponent>(TEXT("TargetTagRequirements"));
	TagRequirements->OngoingTagRequirements.IgnoreTags.AddTag(TAG_Arcane_Vision);
	GEComponents.Add(TagRequirements);
}
```

- [ ] **Step 5: Run the test to verify it passes**

Run the build command, then the test command with `ThroughArcaneEyes.GAS.ManaEffects`.
Expected: `Test Completed. Result={Success}` for that test (read it out of `Saved/Logs/ThroughArcaneEyes.log`, not stdout).

- [ ] **Step 6: Commit**

```powershell
git add Source/ThroughArcaneEyes/Public/GAS/TaeManaEffects.h Source/ThroughArcaneEyes/Private/GAS/TaeManaEffects.cpp Source/ThroughArcaneEyes/Private/Tests/TaeManaEffectsTest.cpp
git commit -m "[GAS][+] add periodic mana drain and regen effects"
```

---

## Task 4: Arcane Vision drains mana

The hole this milestone exists to close. After this task, surveying has a cost.

**Files:**
- Modify: `Source/ThroughArcaneEyes/Public/GAS/GA_SpectralShift.h:22-27`
- Modify: `Source/ThroughArcaneEyes/Private/GAS/GA_SpectralShift.cpp`

**Interfaces:**
- Consumes: `UTaeManaDrainEffect`, `UTaeManaEffectBase::MagnitudePerPeriod`, `TAG_Data_ManaRate` (Tasks 1 and 3).
- Produces: an `EditDefaultsOnly float ArcaneDrainPerSecond` on `UGA_SpectralShift`; mana falling while `Arcane.Vision` is held.

- [ ] **Step 1: Add the properties**

In `Public/GAS/GA_SpectralShift.h`, in the `protected` section after `ArcaneInputContext`:

```cpp
	// Mana drained per second while Arcane Vision is active. Tune in BP_GA_SpectralShift.
	UPROPERTY(EditDefaultsOnly, Category = "Tae", meta = (ClampMin = "0.0"))
	float ArcaneDrainPerSecond = 4.f;

	// Defaults to UTaeManaDrainEffect; overridable in BP_GA_SpectralShift
	UPROPERTY(EditDefaultsOnly, Category = "Tae")
	TSubclassOf<UGameplayEffect> DrainEffectClass;

private:
	FActiveGameplayEffectHandle DrainHandle;
```

Add `class UGameplayEffect;` to the forward declarations at the top, and include `"ActiveGameplayEffectHandle.h"`.

- [ ] **Step 2: Default the effect class**

In `Private/GAS/GA_SpectralShift.cpp`, add the includes and extend the constructor:

```cpp
#include "GAS/TaeManaEffects.h"
#include "AbilitySystemComponent.h"

UGA_SpectralShift::UGA_SpectralShift()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	DrainEffectClass = UTaeManaDrainEffect::StaticClass();
}
```

- [ ] **Step 3: Apply the drain on activation**

In `ActivateAbility`, after the `SetArcaneActive(true)` block:

```cpp
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (ASC && DrainEffectClass)
	{
		const FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		const FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(DrainEffectClass, 1.f, Context);
		if (Spec.IsValid())
		{
			// Negative — this is a drain. Per-period, not per-second.
			Spec.Data->SetSetByCallerMagnitude(
				TAG_Data_ManaRate, UTaeManaEffectBase::MagnitudePerPeriod(-ArcaneDrainPerSecond));
			DrainHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
		}
	}
```

- [ ] **Step 4: Remove the drain on end**

In `EndAbility`, before the `Super::EndAbility` call:

```cpp
	if (DrainHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
		{
			ASC->RemoveActiveGameplayEffect(DrainHandle);
		}
		DrainHandle.Invalidate();
	}
```

- [ ] **Step 5: Build and verify in PIE**

Run the build command. Expected: `Result: Succeeded`.

Open `ThroughArcaneEyes.uproject`, PIE into `WorldNull`, and hold Arcane Vision.
Expected: the HUD mana number falls by roughly 4 per second and stops falling when Arcane is toggled off.

- [ ] **Step 6: Commit**

```powershell
git add Source/ThroughArcaneEyes/Public/GAS/GA_SpectralShift.h Source/ThroughArcaneEyes/Private/GAS/GA_SpectralShift.cpp
git commit -m "[GAS][+] drain mana while arcane vision is active"
```

---

## Task 5: Exhaustion ejects the player and locks re-entry

**Files:**
- Modify: `Source/ThroughArcaneEyes/Public/GAS/GA_SpectralShift.h`
- Modify: `Source/ThroughArcaneEyes/Private/GAS/GA_SpectralShift.cpp`

**Interfaces:**
- Consumes: `TAG_Arcane_Exhausted` (Task 1), the drain from Task 4.
- Produces: Arcane Vision cancelling itself at zero mana and refusing to reactivate until recovery.

- [ ] **Step 1: Declare the listener**

In `Public/GAS/GA_SpectralShift.h`, in the private section:

```cpp
	void OnExhaustionChanged(const FGameplayTag Tag, int32 NewCount);

	FDelegateHandle ExhaustionHandle;
```

- [ ] **Step 2: Block activation while exhausted**

In the constructor in `Private/GAS/GA_SpectralShift.cpp`:

```cpp
	// Blocks activation. Cancelling a running ability needs the listener below — blocked tags do not
	// interrupt an ability that is already active.
	ActivationBlockedTags.AddTag(TAG_Arcane_Exhausted);
```

- [ ] **Step 3: Register and unregister the listener**

At the end of `ActivateAbility`:

```cpp
	if (ASC)
	{
		ExhaustionHandle = ASC->RegisterGameplayTagEvent(TAG_Arcane_Exhausted, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &UGA_SpectralShift::OnExhaustionChanged);
	}
```

In `EndAbility`, beside the drain removal:

```cpp
	if (ExhaustionHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
		{
			ASC->RegisterGameplayTagEvent(TAG_Arcane_Exhausted, EGameplayTagEventType::NewOrRemoved).Remove(ExhaustionHandle);
		}
		ExhaustionHandle.Reset();
	}
```

And the handler, mirroring `UGA_GrowRoot::OnArcaneVisionChanged`:

```cpp
void UGA_SpectralShift::OnExhaustionChanged(const FGameplayTag Tag, const int32 NewCount)
{
	// Running dry ejects the player to Forest. UGA_GrowRoot cancels itself off the resulting
	// Arcane.Vision loss, so the channel needs no exhaustion logic of its own.
	if (NewCount > 0)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}
```

- [ ] **Step 4: Build and verify the cascade in PIE**

Run the build command. Expected: `Result: Succeeded`.

PIE into `WorldNull`. Hold Arcane Vision until mana reaches zero.
Expected: Arcane drops on its own, the camera and post-process blend back, pressing the toggle again does nothing, and the log shows no errors.

Then start a growth channel and hold it to zero.
Expected: the channel stops, Arcane drops, and the root keeps its partial `GrowthAlpha`.

- [ ] **Step 5: Commit**

```powershell
git add Source/ThroughArcaneEyes/Public/GAS/GA_SpectralShift.h Source/ThroughArcaneEyes/Private/GAS/GA_SpectralShift.cpp
git commit -m "[GAS][+] eject from arcane vision on exhaustion"
```

---

## Task 6: GrowRoot spends through the effect

Deletes the direct attribute write the milestone exists to remove. This task should make `GA_GrowRoot.cpp` shorter.

**Files:**
- Modify: `Source/ThroughArcaneEyes/Public/GAS/GA_GrowRoot.h:53-55`
- Modify: `Source/ThroughArcaneEyes/Private/GAS/GA_GrowRoot.cpp:116-167`

**Interfaces:**
- Consumes: `UTaeManaDrainEffect`, `UTaeManaEffectBase::MagnitudePerPeriod` (Task 3).
- Produces: growth drain flowing through GAS; `TickGrowth` no longer touching mana.

- [ ] **Step 1: Add the effect handle**

In `Public/GAS/GA_GrowRoot.h`, in the private section beside `GrowthTimerHandle`:

```cpp
	// Defaults to UTaeManaDrainEffect; overridable in BP_GA_GrowRoot
	UPROPERTY(EditDefaultsOnly, Category = "GrowRoot")
	TSubclassOf<UGameplayEffect> DrainEffectClass;

	FActiveGameplayEffectHandle DrainHandle;
```

Add `class UGameplayEffect;` to the forward declarations and include `"ActiveGameplayEffectHandle.h"`.

- [ ] **Step 2: Default the class and apply the drain**

In the constructor in `Private/GAS/GA_GrowRoot.cpp`:

```cpp
	DrainEffectClass = UTaeManaDrainEffect::StaticClass();
```

In `ActivateAbility`, after the growth timer is set:

```cpp
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		if (DrainEffectClass)
		{
			const FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			const FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(DrainEffectClass, 1.f, Context);
			if (Spec.IsValid())
			{
				// Stacks with the vision drain already applied by UGA_SpectralShift
				Spec.Data->SetSetByCallerMagnitude(
					TAG_Data_ManaRate, UTaeManaEffectBase::MagnitudePerPeriod(-ManaCostPerSecond));
				DrainHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
			}
		}
	}
```

Add `#include "GAS/TaeManaEffects.h"` at the top.

- [ ] **Step 3: Delete the direct attribute write**

Replace the whole body of `TickGrowth` with:

```cpp
void UGA_GrowRoot::TickGrowth()
{
	if (!ActivePath)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	ActivePath->AdvanceGrowth(GrowthRate * GrowthTickInterval);

	// Finished — stop rather than burning mana on a full path
	if (ActivePath->GetConnectionState() == ETaeConnectionState::Restored)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}
```

Running out of mana is no longer checked here: exhaustion drops `Arcane.Vision`, and the existing listener at `GA_GrowRoot.cpp:107` cancels the channel. Remove the now-unused `#include "GAS/TaeManaAttributeSet.h"`.

- [ ] **Step 4: Remove the drain on end**

In `EndAbility`, beside the timer clear:

```cpp
	if (DrainHandle.IsValid())
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			ASC->RemoveActiveGameplayEffect(DrainHandle);
		}
		DrainHandle.Invalidate();
	}
```

- [ ] **Step 5: Build and verify in PIE**

Run the build command, then the test command (the M1 growth tests must still pass).
Expected: `Result: Succeeded`, and no `Result={Fail` in `Saved/Logs/ThroughArcaneEyes.log`.

PIE into `WorldNull`. Enter Arcane, stand at an anchor, and channel.
Expected: mana falls at roughly 16 per second while channelling (4 vision + 12 growth) and returns to roughly 4 per second on release, with growth progress preserved.

- [ ] **Step 6: Commit**

```powershell
git add Source/ThroughArcaneEyes/Public/GAS/GA_GrowRoot.h Source/ThroughArcaneEyes/Private/GAS/GA_GrowRoot.cpp
git commit -m "[GAS][*] spend growth mana through a gameplay effect"
```

---

## Task 7: The grove

**Files:**
- Create: `Source/ThroughArcaneEyes/Public/World/TaeGroveComponent.h`
- Create: `Source/ThroughArcaneEyes/Private/World/TaeGroveComponent.cpp`
- Create: `Source/ThroughArcaneEyes/Private/Tests/TaeGroveRegenTest.cpp`

**Interfaces:**
- Consumes: `UTaeManaRegenEffect`, `UTaeManaEffectBase::MagnitudePerPeriod`, `TAG_Data_ManaRate` (Tasks 1 and 3).
- Produces: `static float UTaeGroveComponent::RegenRateForArea(float AreaSqM, const UCurveFloat* Curve)`; mana regenerating inside a placed grove.

- [ ] **Step 1: Write the failing test**

Create `Source/ThroughArcaneEyes/Private/Tests/TaeGroveRegenTest.cpp`:

```cpp
// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "Misc/AutomationTest.h"
#include "World/TaeGroveComponent.h"
#include "Curves/CurveFloat.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTaeGroveRegenTest,
	"ThroughArcaneEyes.World.GroveRegen",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTaeGroveRegenTest::RunTest(const FString& Parameters)
{
	// No curve means no regen, rather than a crash or an accidental free refill
	TestEqual(TEXT("null curve yields nothing"),
		UTaeGroveComponent::RegenRateForArea(100.f, nullptr), 0.f);

	UCurveFloat* Curve = NewObject<UCurveFloat>();
	Curve->FloatCurve.AddKey(50.f, 4.f);
	Curve->FloatCurve.AddKey(200.f, 10.f);

	// Authored points read back exactly
	TestEqual(TEXT("small grove"),
		UTaeGroveComponent::RegenRateForArea(50.f, Curve), 4.f);
	TestEqual(TEXT("large grove"),
		UTaeGroveComponent::RegenRateForArea(200.f, Curve), 10.f);

	// Outside the authored range the curve clamps rather than extrapolating away
	TestEqual(TEXT("below the first key clamps"),
		UTaeGroveComponent::RegenRateForArea(0.f, Curve), 4.f);
	TestEqual(TEXT("above the last key clamps"),
		UTaeGroveComponent::RegenRateForArea(1000.f, Curve), 10.f);

	// A degenerate footprint cannot produce negative regen
	TestTrue(TEXT("zero area is not negative"),
		UTaeGroveComponent::RegenRateForArea(0.f, Curve) >= 0.f);

	return true;
}

#endif
```

- [ ] **Step 2: Run the test to verify it fails**

Run the build command. Expected: FAIL — `World/TaeGroveComponent.h` does not exist.

- [ ] **Step 3: Write the header**

Create `Source/ThroughArcaneEyes/Public/World/TaeGroveComponent.h`:

```cpp
// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "ActiveGameplayEffectHandle.h"
#include "TaeGroveComponent.generated.h"

class UCurveFloat;
class UGameplayEffect;
// Actor headers pick this up transitively; a component header may not
class FDataValidationContext;

// A patch of living land. Standing inside it regenerates mana at a rate derived from its own footprint,
// so a larger restoration pays more. Regen is inhibited while Arcane Vision is active.
//
// A component rather than an actor: M2 hosts it on a hand-placed BP_Grove, and M4 attaches the same
// component to whatever ends up owning restoration state.
UCLASS(ClassGroup = (Tae), meta = (BlueprintSpawnableComponent))
class THROUGHARCANEEYES_API UTaeGroveComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	UTaeGroveComponent();

	// Horizontal footprint in square metres, from this component's own scaled box extent
	UFUNCTION(BlueprintPure, Category = "Tae|Grove")
	float GetFootprintAreaSqM() const;

	// Pure curve lookup. Static and world-free so it can be tested directly.
	// A null curve yields zero — a grove with no authored curve grants nothing.
	static float RegenRateForArea(float AreaSqM, const UCurveFloat* Curve);

	UFUNCTION(BlueprintPure, Category = "Tae|Grove")
	float GetRegenPerSecond() const;

protected:
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif

private:
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Mana per second by footprint area in m². Assign Curve_GroveRegen in BP_Grove.
	UPROPERTY(EditAnywhere, Category = "Tae|Grove")
	TObjectPtr<UCurveFloat> RegenCurve;

	// Defaults to UTaeManaRegenEffect; overridable in BP_Grove
	UPROPERTY(EditDefaultsOnly, Category = "Tae|Grove")
	TSubclassOf<UGameplayEffect> RegenEffectClass;

	// One entry per occupant, so overlapping pawns cannot remove each other's regen
	TMap<TWeakObjectPtr<AActor>, FActiveGameplayEffectHandle> ActiveRegen;
};
```

- [ ] **Step 4: Write the implementation**

Create `Source/ThroughArcaneEyes/Private/World/TaeGroveComponent.cpp`:

```cpp
// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "World/TaeGroveComponent.h"
#include "GAS/TaeGASTypes.h"
#include "GAS/TaeManaEffects.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Curves/CurveFloat.h"
#include "ThroughArcaneEyes.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

namespace
{
	// Unreal units are centimetres; curves are authored in metres
	constexpr float UuPerMetre = 100.f;
}

UTaeGroveComponent::UTaeGroveComponent()
{
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionResponseToAllChannels(ECR_Overlap);
	SetGenerateOverlapEvents(true);
	RegenEffectClass = UTaeManaRegenEffect::StaticClass();
}

float UTaeGroveComponent::GetFootprintAreaSqM() const
{
	// Scaled extent is a half-size, so the full footprint is twice each axis
	const FVector Extent = GetScaledBoxExtent();
	const float WidthM = (Extent.X * 2.f) / UuPerMetre;
	const float DepthM = (Extent.Y * 2.f) / UuPerMetre;
	return FMath::Max(WidthM * DepthM, 0.f);
}

float UTaeGroveComponent::RegenRateForArea(const float AreaSqM, const UCurveFloat* Curve)
{
	if (!Curve)
	{
		return 0.f;
	}

	return FMath::Max(Curve->GetFloatValue(AreaSqM), 0.f);
}

float UTaeGroveComponent::GetRegenPerSecond() const
{
	return RegenRateForArea(GetFootprintAreaSqM(), RegenCurve);
}

void UTaeGroveComponent::BeginPlay()
{
	Super::BeginPlay();

	OnComponentBeginOverlap.AddDynamic(this, &UTaeGroveComponent::HandleBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &UTaeGroveComponent::HandleEndOverlap);
}

void UTaeGroveComponent::HandleBeginOverlap(UPrimitiveComponent*, AActor* OtherActor,
	UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	const IAbilitySystemInterface* AsInterface = Cast<IAbilitySystemInterface>(OtherActor);
	if (!AsInterface || ActiveRegen.Contains(OtherActor))
	{
		return;
	}

	UAbilitySystemComponent* ASC = AsInterface->GetAbilitySystemComponent();
	if (!ASC || !RegenEffectClass)
	{
		return;
	}

	const FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	const FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(RegenEffectClass, 1.f, Context);
	if (!Spec.IsValid())
	{
		return;
	}

	// Positive — this restores. Per-period, not per-second.
	Spec.Data->SetSetByCallerMagnitude(
		TAG_Data_ManaRate, UTaeManaEffectBase::MagnitudePerPeriod(GetRegenPerSecond()));
	ActiveRegen.Add(OtherActor, ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data));
}

void UTaeGroveComponent::HandleEndOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32)
{
	FActiveGameplayEffectHandle Handle;
	if (!ActiveRegen.RemoveAndCopyValue(OtherActor, Handle) || !Handle.IsValid())
	{
		return;
	}

	const IAbilitySystemInterface* AsInterface = Cast<IAbilitySystemInterface>(OtherActor);
	if (UAbilitySystemComponent* ASC = AsInterface ? AsInterface->GetAbilitySystemComponent() : nullptr)
	{
		ASC->RemoveActiveGameplayEffect(Handle);
	}
}

#if WITH_EDITOR
EDataValidationResult UTaeGroveComponent::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (!RegenCurve)
	{
		Context.AddError(NSLOCTEXT("TaeValidation", "NoRegenCurve",
			"TaeGroveComponent: RegenCurve is not assigned — this grove grants no mana."));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif
```

- [ ] **Step 5: Run the test to verify it passes**

Run the build command, then the test command with `ThroughArcaneEyes.World.GroveRegen`.
Expected: `Test Completed. Result={Success}` for that test (read it out of `Saved/Logs/ThroughArcaneEyes.log`, not stdout).

- [ ] **Step 6: Commit**

```powershell
git add Source/ThroughArcaneEyes/Public/World/TaeGroveComponent.h Source/ThroughArcaneEyes/Private/World/TaeGroveComponent.cpp Source/ThroughArcaneEyes/Private/Tests/TaeGroveRegenTest.cpp
git commit -m "[World][+] add grove component granting area-scaled mana regen"
```

---

## Task 8: ViewModel flow state

The HUD needs to say what mana is doing, not just how much there is. Flow is counted in the ViewModel so the cue notifies stay stateless — which they must be, since their overrides are `const`.

**Files:**
- Modify: `Source/ThroughArcaneEyes/Public/UI/TaeHudViewModel.h`
- Modify: `Source/ThroughArcaneEyes/Private/UI/TaeHudViewModel.cpp`
- Create: `Source/ThroughArcaneEyes/Private/Tests/TaeManaFlowTest.cpp`

**Interfaces:**
- Consumes: nothing.
- Produces: `ETaeManaFlow`; `static ETaeManaFlow UTaeHudViewModel::ResolveManaFlow(int32 DrainCount, int32 RegenCount)`; `BeginManaFlow(ETaeManaFlow)` / `EndManaFlow(ETaeManaFlow)`; `SetMaxMana(float)`; `SetExhausted(bool)`.

- [ ] **Step 1: Write the failing test**

Create `Source/ThroughArcaneEyes/Private/Tests/TaeManaFlowTest.cpp`:

```cpp
// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "Misc/AutomationTest.h"
#include "UI/TaeHudViewModel.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTaeManaFlowTest,
	"ThroughArcaneEyes.UI.ManaFlow",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTaeManaFlowTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("nothing active is idle"),
		UTaeHudViewModel::ResolveManaFlow(0, 0) == ETaeManaFlow::Idle);

	TestTrue(TEXT("one drain is draining"),
		UTaeHudViewModel::ResolveManaFlow(1, 0) == ETaeManaFlow::Draining);

	TestTrue(TEXT("one regen is regenerating"),
		UTaeHudViewModel::ResolveManaFlow(0, 1) == ETaeManaFlow::Regenerating);

	// Channelling adds a second drain on top of the vision drain — one ending must not clear the state
	TestTrue(TEXT("two drains still draining"),
		UTaeHudViewModel::ResolveManaFlow(2, 0) == ETaeManaFlow::Draining);

	// Regen is inhibited during Arcane, but if both are ever reported, spending wins visually
	TestTrue(TEXT("drain outranks regen"),
		UTaeHudViewModel::ResolveManaFlow(1, 1) == ETaeManaFlow::Draining);

	// Counts cannot go negative and drive a bogus state
	TestTrue(TEXT("negative counts read as idle"),
		UTaeHudViewModel::ResolveManaFlow(-1, 0) == ETaeManaFlow::Idle);

	return true;
}

#endif
```

- [ ] **Step 2: Run the test to verify it fails**

Run the build command. Expected: FAIL — `ETaeManaFlow` is undeclared.

- [ ] **Step 3: Extend the ViewModel header**

In `Public/UI/TaeHudViewModel.h`, above the class:

```cpp
// What mana is doing right now — drives the HUD bar tint
UENUM(BlueprintType)
enum class ETaeManaFlow : uint8
{
	Idle,
	Draining,
	Regenerating
};
```

Inside the class, extend the public section:

```cpp
	void SetMaxMana(float NewMaxMana);
	void SetExhausted(bool bNewExhausted);

	// Called by the mana cue notifies. Counted rather than toggled, because two drains can be active at
	// once (vision plus growth) and the notifies themselves are stateless.
	void BeginManaFlow(ETaeManaFlow Flow);
	void EndManaFlow(ETaeManaFlow Flow);

	// Pure resolution of the two counters into one displayed state. Draining outranks regenerating.
	static ETaeManaFlow ResolveManaFlow(int32 DrainCount, int32 RegenCount);
```

Extend the raw and presentation blocks:

```cpp
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Tae")
	float MaxMana = 100.f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Tae")
	float ManaPercent = 0.f;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Tae")
	bool bExhausted = false;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Tae")
	ETaeManaFlow ManaFlow = ETaeManaFlow::Idle;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Tae|UI")
	FLinearColor ManaBarTint = FLinearColor::White;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Tae|UI")
	ESlateVisibility ExhaustedVisibility = ESlateVisibility::Collapsed;
```

and a private counter block:

```cpp
private:
	void RefreshManaFlow();
	void RefreshManaPercent();

	int32 DrainCount = 0;
	int32 RegenCount = 0;
```

- [ ] **Step 4: Implement it**

In `Private/UI/TaeHudViewModel.cpp`:

```cpp
ETaeManaFlow UTaeHudViewModel::ResolveManaFlow(const int32 InDrainCount, const int32 InRegenCount)
{
	if (InDrainCount > 0)
	{
		return ETaeManaFlow::Draining;
	}
	if (InRegenCount > 0)
	{
		return ETaeManaFlow::Regenerating;
	}
	return ETaeManaFlow::Idle;
}

void UTaeHudViewModel::SetMaxMana(const float NewMaxMana)
{
	UE_MVVM_SET_PROPERTY_VALUE(MaxMana, NewMaxMana);
	RefreshManaPercent();
}

void UTaeHudViewModel::SetExhausted(const bool bNewExhausted)
{
	UE_MVVM_SET_PROPERTY_VALUE(bExhausted, bNewExhausted);
	UE_MVVM_SET_PROPERTY_VALUE(ExhaustedVisibility,
		bNewExhausted ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void UTaeHudViewModel::BeginManaFlow(const ETaeManaFlow Flow)
{
	if (Flow == ETaeManaFlow::Draining) { ++DrainCount; }
	else if (Flow == ETaeManaFlow::Regenerating) { ++RegenCount; }
	RefreshManaFlow();
}

void UTaeHudViewModel::EndManaFlow(const ETaeManaFlow Flow)
{
	if (Flow == ETaeManaFlow::Draining) { DrainCount = FMath::Max(DrainCount - 1, 0); }
	else if (Flow == ETaeManaFlow::Regenerating) { RegenCount = FMath::Max(RegenCount - 1, 0); }
	RefreshManaFlow();
}

void UTaeHudViewModel::RefreshManaFlow()
{
	const ETaeManaFlow NewFlow = ResolveManaFlow(DrainCount, RegenCount);
	UE_MVVM_SET_PROPERTY_VALUE(ManaFlow, NewFlow);

	FLinearColor NewTint = FLinearColor::White;
	if (NewFlow == ETaeManaFlow::Draining)
	{
		NewTint = FLinearColor(1.f, 0.45f, 0.2f);
	}
	else if (NewFlow == ETaeManaFlow::Regenerating)
	{
		NewTint = FLinearColor(0.4f, 1.f, 0.5f);
	}
	UE_MVVM_SET_PROPERTY_VALUE(ManaBarTint, NewTint);
}

void UTaeHudViewModel::RefreshManaPercent()
{
	const float NewPercent = MaxMana > 0.f ? FMath::Clamp(Mana / MaxMana, 0.f, 1.f) : 0.f;
	UE_MVVM_SET_PROPERTY_VALUE(ManaPercent, NewPercent);
}
```

Add `RefreshManaPercent();` to the end of the existing `SetMana`.

- [ ] **Step 5: Run the test to verify it passes**

Run the build command, then the test command with `ThroughArcaneEyes.UI.ManaFlow`.
Expected: `Test Completed. Result={Success}` for that test (read it out of `Saved/Logs/ThroughArcaneEyes.log`, not stdout).

- [ ] **Step 6: Commit**

```powershell
git add Source/ThroughArcaneEyes/Public/UI/TaeHudViewModel.h Source/ThroughArcaneEyes/Private/UI/TaeHudViewModel.cpp Source/ThroughArcaneEyes/Private/Tests/TaeManaFlowTest.cpp
git commit -m "[UI][+] add mana flow and exhaustion state to the hud view model"
```

---

## Task 9: Cue notifies and controller bindings

**Files:**
- Create: `Source/ThroughArcaneEyes/Public/GAS/TaeCueNotifies.h`
- Create: `Source/ThroughArcaneEyes/Private/GAS/TaeCueNotifies.cpp`
- Modify: `Source/ThroughArcaneEyes/Private/Character/TaePlayerController.cpp:79-95`

**Interfaces:**
- Consumes: `UTaeHudViewModel::BeginManaFlow` / `EndManaFlow` / `SetMaxMana` / `SetExhausted` (Task 8); `UTaeArcaneSubsystem::FlashVignette` (existing).
- Produces: `UTaeCueNotify_ManaDrain`, `UTaeCueNotify_ManaRegen`, `UTaeCueNotify_ArcaneExhausted` — the parent classes the `GC_*` Blueprints in Task 10 derive from.

- [ ] **Step 1: Write the notify header**

Create `Source/ThroughArcaneEyes/Public/GAS/TaeCueNotifies.h`:

```cpp
// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "TaeCueNotifies.generated.h"

// Mana cue notifies.
//
// These classes hold the logic; the Blueprint assets that derive from them exist only so the cue manager
// can find them — it builds its registry from an asset registry scan, so a pure C++ class is never
// registered. The asset NAME derives the tag (GC_Mana_Drain -> GameplayCue.Mana.Drain), so GameplayCueTag
// is deliberately left unset here. See the M2 spec §6.
//
// Every override is const: notifies are stateless. Counting lives in UTaeHudViewModel.

UCLASS()
class THROUGHARCANEEYES_API UTaeCueNotify_ManaDrain : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual bool WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
};

UCLASS()
class THROUGHARCANEEYES_API UTaeCueNotify_ManaRegen : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual bool WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
	virtual bool OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
};

UCLASS()
class THROUGHARCANEEYES_API UTaeCueNotify_ArcaneExhausted : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
};
```

- [ ] **Step 2: Write the implementation**

Create `Source/ThroughArcaneEyes/Private/GAS/TaeCueNotifies.cpp`:

```cpp
// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "GAS/TaeCueNotifies.h"
#include "Core/TaeGameInstance.h"
#include "Core/TaeArcaneSubsystem.h"
#include "UI/TaeHudViewModel.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

namespace
{
	// Cues reach the UI the same way everything else does — never by touching a widget directly
	UTaeHudViewModel* FindViewModel(const AActor* Target)
	{
		const UWorld* World = Target ? Target->GetWorld() : nullptr;
		const UTaeGameInstance* GI = World ? World->GetGameInstance<UTaeGameInstance>() : nullptr;
		return GI ? GI->GetHudViewModel() : nullptr;
	}
}

bool UTaeCueNotify_ManaDrain::WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters&) const
{
	if (UTaeHudViewModel* VM = FindViewModel(MyTarget))
	{
		VM->BeginManaFlow(ETaeManaFlow::Draining);
	}
	return true;
}

bool UTaeCueNotify_ManaDrain::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters&) const
{
	if (UTaeHudViewModel* VM = FindViewModel(MyTarget))
	{
		VM->EndManaFlow(ETaeManaFlow::Draining);
	}
	return true;
}

bool UTaeCueNotify_ManaRegen::WhileActive_Implementation(AActor* MyTarget, const FGameplayCueParameters&) const
{
	if (UTaeHudViewModel* VM = FindViewModel(MyTarget))
	{
		VM->BeginManaFlow(ETaeManaFlow::Regenerating);
	}
	return true;
}

bool UTaeCueNotify_ManaRegen::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters&) const
{
	if (UTaeHudViewModel* VM = FindViewModel(MyTarget))
	{
		VM->EndManaFlow(ETaeManaFlow::Regenerating);
	}
	return true;
}

bool UTaeCueNotify_ArcaneExhausted::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters&) const
{
	const UWorld* World = MyTarget ? MyTarget->GetWorld() : nullptr;
	if (UTaeArcaneSubsystem* Arcane = World ? World->GetSubsystem<UTaeArcaneSubsystem>() : nullptr)
	{
		Arcane->FlashVignette();
	}
	return true;
}
```

- [ ] **Step 3: Bind MaxMana and exhaustion to the ViewModel**

In `Private/Character/TaePlayerController.cpp`, in `SetPawn` after the existing Mana binding:

```cpp
	// MaxMana attribute → MaxMana
	ASC->GetGameplayAttributeValueChangeDelegate(UTaeManaAttributeSet::GetMaxManaAttribute())
		.AddWeakLambda(VM, [VM](const FOnAttributeChangeData& Data)
		{
			VM->SetMaxMana(Data.NewValue);
		});

	// Exhausted tag → bExhausted
	ASC->RegisterGameplayTagEvent(TAG_Arcane_Exhausted, EGameplayTagEventType::AnyCountChange)
		.AddWeakLambda(VM, [VM](const FGameplayTag&, int32 Count)
		{
			VM->SetExhausted(Count > 0);
		});
```

and extend the immediate push block below it:

```cpp
	VM->SetMaxMana(ASC->GetNumericAttribute(UTaeManaAttributeSet::GetMaxManaAttribute()));
	VM->SetExhausted(ASC->HasMatchingGameplayTag(TAG_Arcane_Exhausted));
```

Note: push `SetMaxMana` **before** the existing `SetMana` call so the first percent calculation uses a real maximum.

- [ ] **Step 4: Build and run the full suite**

Run the build command, then the test command.
Expected: `Result: Succeeded`, and no `Result={Fail` in `Saved/Logs/ThroughArcaneEyes.log`.

- [ ] **Step 5: Commit**

```powershell
git add Source/ThroughArcaneEyes/Public/GAS/TaeCueNotifies.h Source/ThroughArcaneEyes/Private/GAS/TaeCueNotifies.cpp Source/ThroughArcaneEyes/Private/Character/TaePlayerController.cpp
git commit -m "[GAS][+] add mana cue notifies and bind exhaustion to the hud"
```

---

## Task 10: Editor assets via Python

**Files:**
- Modify: `ThroughArcaneEyes.uproject`
- Create: `Tools/Python/tae_m2_assets.py`

**Interfaces:**
- Consumes: `UTaeCueNotify_ManaDrain` / `_ManaRegen` / `_ArcaneExhausted` (Task 9), `UTaeGroveComponent` (Task 7).
- Produces: `/Game/GAS/Cues/GC_Mana_Drain`, `GC_Mana_Regen`, `GC_Arcane_Exhausted`; `/Game/World/Curve_GroveRegen`; `/Game/World/BP_Grove`; one grove placed in `WorldNull`.

- [ ] **Step 1: Enable the Python plugin**

In `ThroughArcaneEyes.uproject`, add to the `Plugins` array:

```json
		{
			"Name": "PythonScriptPlugin",
			"Enabled": true
		}
```

- [ ] **Step 2: Write the asset script**

Create `Tools/Python/tae_m2_assets.py`:

```python
# Copyright (c) 2026 Helen Allien Poe. Source available - see LICENSE.
"""Creates the M2 editor assets and places one grove in WorldNull.

Run headless:
  UnrealEditor-Cmd.exe <project>.uproject -run=pythonscript -script="<this file>" -unattended -nopause -nosplash

Cue Blueprint names are load-bearing: the cue manager derives the tag from the asset name
(GC_Mana_Drain -> GameplayCue.Mana.Drain). Do not rename them.
"""

import unreal

CUE_PATH = "/Game/GAS/Cues"
WORLD_PATH = "/Game/World"
MAP_PATH = "/Game/Maps/WorldNull"

assets = unreal.AssetToolsHelpers.get_asset_tools()


def make_blueprint(name, package_path, parent_class):
    full = "{}/{}".format(package_path, name)
    if unreal.EditorAssetLibrary.does_asset_exist(full):
        unreal.log("exists, skipping: {}".format(full))
        return unreal.EditorAssetLibrary.load_asset(full)

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    asset = assets.create_asset(name, package_path, unreal.Blueprint, factory)
    unreal.EditorAssetLibrary.save_asset(full)
    unreal.log("created: {}".format(full))
    return asset


def make_regen_curve():
    full = "{}/{}".format(WORLD_PATH, "Curve_GroveRegen")
    if unreal.EditorAssetLibrary.does_asset_exist(full):
        return unreal.EditorAssetLibrary.load_asset(full)

    curve = assets.create_asset("Curve_GroveRegen", WORLD_PATH, unreal.CurveFloat, unreal.CurveFloatFactory())
    # Footprint area in square metres -> mana per second. Starting values; tune in-editor.
    curve.float_curve.add_key(50.0, 4.0)
    curve.float_curve.add_key(200.0, 10.0)
    unreal.EditorAssetLibrary.save_asset(full)
    unreal.log("created: {}".format(full))
    return curve


def main():
    make_blueprint("GC_Mana_Drain", CUE_PATH, unreal.TaeCueNotify_ManaDrain)
    make_blueprint("GC_Mana_Regen", CUE_PATH, unreal.TaeCueNotify_ManaRegen)
    make_blueprint("GC_Arcane_Exhausted", CUE_PATH, unreal.TaeCueNotify_ArcaneExhausted)

    make_regen_curve()
    make_blueprint("BP_Grove", WORLD_PATH, unreal.Actor)

    level_subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    level_subsystem.load_level(MAP_PATH)

    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    grove_bp = unreal.EditorAssetLibrary.load_asset("{}/{}".format(WORLD_PATH, "BP_Grove"))
    actor_subsystem.spawn_actor_from_object(grove_bp, unreal.Vector(0.0, 0.0, 100.0))

    level_subsystem.save_current_level()
    unreal.log("M2 assets done")


main()
```

- [ ] **Step 3: Run the script**

```powershell
& 'D:\EpicGames\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' "D:\PetProjects\ThroughArcaneEyes\ThroughArcaneEyes.uproject" -run=pythonscript -script="D:\PetProjects\ThroughArcaneEyes\Tools\Python\tae_m2_assets.py" -unattended -nopause -nosplash
```

Expected: log lines `created: /Game/GAS/Cues/GC_Mana_Drain` (and the rest), ending in `M2 assets done`, with exit code 0.

**Amended during implementation (2026-08-11).** The script grew past what this step describes: it also
adds the `UTaeGroveComponent` to `BP_Grove` (via `SubobjectDataSubsystem`), sizes it, and assigns the
curve — so `BP_Grove` needs no manual finishing. Two things it does *not* do:

- **The regen curve is imported from a temporary CSV, not set directly.** `UCurveFloat::FloatCurve` is a
  plain `UPROPERTY()` with no edit specifiers, so the Python scripting layer (which exposes only
  `CPF_Edit` properties) cannot reach it. `CSVImportFactory` is the supported route.
- **It does not place the grove in the level.** `EditorActorSubsystem.spawn_actor_from_object` crashes
  the commandlet — `EXCEPTION_ACCESS_VIOLATION` inside `EditorFramework.dll`, immediately after
  `load_level` returns. Actor placement wants a level-editor context `-run=pythonscript` never builds.
  Placement moved to the editor handoff, which Task 11 covers.

- [ ] **Step 4: Verify cue registration in the editor**

In the editor console run `GameplayCue.PrintLoadedGameplayCueNotifyClasses`.

Expected: the output lists all three `GC_*` classes. If any is missing, its asset name does not derive a registered tag — check the name against the table in spec §6.

- [ ] **Step 5: Commit**

The level is not touched here — it changes in Task 11 along with the placement.

```powershell
git add ThroughArcaneEyes.uproject Tools/Python/tae_m2_assets.py Content/GAS/Cues Content/World
git commit -m "[Config][+] enable python plugin and script the M2 editor assets"
```

---

## Task 11: The editor pass — grove placement and HUD mana bar

Everything Python cannot do. Written as a handoff document so it can be executed and verified
independently, in the same style as the M1 Task 8 handoff — which is why it lives beside that one in
`docs/superpowers/plans/` rather than in `docs/issues/` as originally planned.

Its scope grew past the HUD once Task 10's spawn crash pushed level placement here, so it also carries
the verification and tuning steps that were split across Tasks 10 and 12.

**Files:**
- Create: [`docs/superpowers/plans/2026-08-11-m2-editor-handoff.md`](2026-08-11-m2-editor-handoff.md) — **written**
- Modify: `Content/Maps/WorldNull.umap` (in-editor — place `BP_Grove`)
- Modify: `Content/UI/Widgets/WBP_HUD.uasset` (in-editor)

**Interfaces:**
- Consumes: `UTaeHudViewModel::ManaPercent` / `ManaBarTint` / `ExhaustedVisibility` (Task 8); `BP_Grove` (Task 10).
- Produces: a placed grove and a HUD that shows the economy.

- [x] **Step 1: Write the handoff document**

- [ ] **Step 2: Do the editor work**

Follow [the handoff](2026-08-11-m2-editor-handoff.md) steps 1–3.

- [ ] **Step 3: Verify**

Handoff step 4 — the ten-row PIE table. Row 9 (regen resuming without re-entering the volume) is the
one that proves the ongoing-tag-requirement design rather than merely exercising it.

- [ ] **Step 4: Commit**

```powershell
git add docs/superpowers/plans/2026-08-11-m2-editor-handoff.md Content/UI/Widgets/WBP_HUD.uasset Content/Maps/WorldNull.umap Content/World/BP_Grove.uasset
git commit -m "[UI][+] show mana flow and exhaustion on the hud"
```

---

## Task 12: Tuning pass and documentation

The spec requires the rates to be tuned before M3 content is built on them, and the gate clip is the tuning target.

**Files:**
- Modify: `README.md`
- Modify: `AGENTS.md:26-39`
- Modify: `Content/GAS/BP_GA_SpectralShift.uasset`, `Content/GAS/BP_GA_GrowRoot.uasset`, `Content/Character/BP_TaeCharacter.uasset` (tuning only, in-editor)

**Interfaces:**
- Consumes: everything above.
- Produces: the M2 gate clip.

- [ ] **Step 1: Tune against the gate**

Play the gate scenario end to end: enter Arcane, channel at a break, run dry, recover in the grove, return and finish.

Adjust in the editor only — no code changes:
- `ArcaneDrainPerSecond` on `BP_GA_SpectralShift`
- `ManaCostPerSecond` and `GrowthRate` on `BP_GA_GrowRoot`
- `ExhaustionRecoveryFraction` on `BP_TaeCharacter`
- `Curve_GroveRegen` keys

Target: surveying an island pair costs a noticeable fraction of the bar, one full connection cannot be grown in a single charge from full, and a recovery visit takes a few seconds rather than a wait.

- [ ] **Step 2: Record the gate clip**

Capture one continuous take: reveal → channel → run dry → ejected with the vignette flash → re-entry refused → walk to the grove → recover → return → finish the connection.

- [ ] **Step 3: Update the docs**

In `README.md`, add to the vocabulary section:

```markdown
- **Grove** — a patch of living land. Standing in one regenerates mana; the rate scales with its footprint.
- **Exhausted** — the state after running dry. Arcane Vision is refused until mana recovers past the floor.
```

In `AGENTS.md`, add two rows to the naming table:

```markdown
| Gameplay Cue notify | `U` + `Tae` + `CueNotify_` | `UTaeCueNotify_ManaDrain` |
| Gameplay Cue asset | `GC_` | `GC_Mana_Drain` — **the name derives the tag** |
```

and a line under Module Architecture:

```markdown
Editor automation lives in `Tools/Python/`, run via `UnrealEditor-Cmd.exe -run=pythonscript`. Requires the `PythonScriptPlugin` entry in the uproject.
```

- [ ] **Step 4: Record the tuned values in the spec**

Append a short "Tuned values (M2 gate)" section to `docs/superpowers/specs/2026-08-11-m2-mana-economy-design.md` §5 recording what the numbers actually landed on, so M3 builds on measured values rather than the proposals.

- [ ] **Step 5: Run the full suite one last time**

Run the build command, then the test command.
Expected: `Result: Succeeded`, and `Found 7 automation tests` with `Result={Success}` on every one — the four new tests `GAS.ManaExhaustion`, `GAS.ManaEffects`, `World.GroveRegen`, `UI.ManaFlow`, plus the three from the baseline: `Core.BlendAlpha`, `Harness.Sanity`, `World.GrowthStep`.

- [ ] **Step 6: Commit**

```powershell
git add README.md AGENTS.md docs/superpowers/specs/2026-08-11-m2-mana-economy-design.md Content/GAS Content/Character
git commit -m "[Docs][*] record M2 vocabulary, cue naming, and tuned rates"
```

---

## Done when

- Arcane Vision drains mana; channelling drains faster; both flow through Gameplay Effects with no direct attribute writes left in either ability.
- Running dry ejects the player to Forest, fires the exhaustion cue, and refuses re-entry until the recovery floor.
- A grove regenerates mana at a rate derived from its footprint, inhibited while Arcane is active.
- The HUD reports amount, flow, and exhaustion through MVVM.
- All three `GC_*` cue Blueprints appear in `GameplayCue.PrintLoadedGameplayCueNotifyClasses`.
- Seven automation tests pass headless — four new, three inherited from M1.
- The gate clip exists.
