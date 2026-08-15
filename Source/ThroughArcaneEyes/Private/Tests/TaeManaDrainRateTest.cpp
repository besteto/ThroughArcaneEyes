// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "Misc/AutomationTest.h"
#include "AbilitySystemComponent.h"
#include "GAS/TaeGASTypes.h"
#include "GAS/TaeManaAttributeSet.h"
#include "GAS/TaeManaEffects.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

#if WITH_DEV_AUTOMATION_TESTS

// The other mana tests cover pure functions. These two need a real world, because the thing worth
// testing is the part no pure function can reach: that a periodic Infinite effect with a SetByCaller
// magnitude actually moves the attribute at the advertised per-second rate once GAS is ticking.
namespace TaeDrainTest
{
	// One tick of slop at the 0.1s period is 0.4 mana. Tolerating that keeps the test stable across
	// timer boundary jitter while still catching a rate that is wrong by any meaningful amount.
	constexpr float Tolerance = 0.5f;

	// A periodic effect's first execution lands a fixed offset after application, so rates are sampled
	// across a window that starts once the effect is already ticking.
	constexpr float SettleSeconds = 1.f;

	// Mirrors the engine's own GAS tests (GameplayEffectTests.cpp:761): sub-tick in fixed steps so
	// periodic effects land, and bump GFrameCounter because GAS caches per-frame state.
	void TickWorld(UWorld* World, float Seconds)
	{
		constexpr float Step = 0.1f;
		while (Seconds > 0.f)
		{
			World->Tick(ELevelTick::LEVELTICK_All, FMath::Min(Seconds, Step));
			Seconds -= Step;
			GFrameCounter++;
		}
	}

	// A bare actor carrying an ASC and our attribute set — deliberately not ATaeCharacter, whose
	// BeginPlay warns about unassigned Blueprint abilities and would dirty the test log.
	UAbilitySystemComponent* MakeCaster(UWorld* World)
	{
		AActor* Actor = World->SpawnActor<AActor>();
		UAbilitySystemComponent* ASC = NewObject<UAbilitySystemComponent>(Actor);
		ASC->RegisterComponent();
		ASC->InitAbilityActorInfo(Actor, Actor);
		ASC->AddSet<UTaeManaAttributeSet>();
		return ASC;
	}

	float GetMana(const UAbilitySystemComponent* ASC)
	{
		return ASC->GetNumericAttribute(UTaeManaAttributeSet::GetManaAttribute());
	}

	// Applies the drain the way UGA_SpectralShift and UGA_GrowRoot both do — negative rate, converted
	// to a per-period magnitude.
	FActiveGameplayEffectHandle ApplyDrain(UAbilitySystemComponent* ASC, const float RatePerSecond)
	{
		const FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		const FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(UTaeManaDrainEffect::StaticClass(), 1.f, Context);
		if (!Spec.IsValid())
		{
			return FActiveGameplayEffectHandle();
		}

		Spec.Data->SetSetByCallerMagnitude(
			TAG_Data_ManaRate, UTaeManaEffectBase::MagnitudePerPeriod(-RatePerSecond));
		return ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
	}

	// Scoped world matching the engine's setup/teardown so the test leaves no context behind.
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTaeManaDrainRateTest,
	"ThroughArcaneEyes.GAS.ManaDrainRate",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTaeManaDrainRateTest::RunTest(const FString& Parameters)
{
	using namespace TaeDrainTest;

	FScopedTestWorld Scope;
	UAbilitySystemComponent* ASC = MakeCaster(Scope.World);

	TestEqual(TEXT("mana starts full"), GetMana(ASC), 100.f);

	// The default UGA_SpectralShift::ArcaneDrainPerSecond
	const FActiveGameplayEffectHandle Drain = ApplyDrain(ASC, 4.f);
	TestTrue(TEXT("the drain applied"), Drain.IsValid());

	// Sampled across a settled window rather than from application. There is a fixed ~0.2s offset
	// before a periodic effect's first execution, which would otherwise be scored as a rate error.
	TickWorld(Scope.World, SettleSeconds);
	const float Before = GetMana(ASC);

	TickWorld(Scope.World, 2.f);
	TestEqual(TEXT("holding vision spends its per-second rate"), Before - GetMana(ASC), 8.f, Tolerance);

	// Dropping out of Arcane must stop the spend, not merely slow it
	ASC->RemoveActiveGameplayEffect(Drain);
	const float AfterRelease = GetMana(ASC);

	TickWorld(Scope.World, 2.f);
	TestEqual(TEXT("removing the drain stops the spend"), GetMana(ASC), AfterRelease, KINDA_SMALL_NUMBER);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTaeManaDrainStackingTest,
	"ThroughArcaneEyes.GAS.ManaDrainStacking",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTaeManaDrainStackingTest::RunTest(const FString& Parameters)
{
	using namespace TaeDrainTest;

	FScopedTestWorld Scope;
	UAbilitySystemComponent* ASC = MakeCaster(Scope.World);

	// Channelling costs vision plus growth at once. Two applications of the same effect class must
	// tick independently and add up — if they collapsed into one, growing would be free.
	const FActiveGameplayEffectHandle Vision = ApplyDrain(ASC, 4.f);
	const FActiveGameplayEffectHandle Growth = ApplyDrain(ASC, 12.f);
	TestTrue(TEXT("both drains applied"), Vision.IsValid() && Growth.IsValid());

	TickWorld(Scope.World, SettleSeconds);
	const float Before = GetMana(ASC);

	TickWorld(Scope.World, 2.f);
	TestEqual(TEXT("channelling spends both rates together"), Before - GetMana(ASC), 32.f, Tolerance);

	// Releasing the channel leaves vision running at its own rate
	ASC->RemoveActiveGameplayEffect(Growth);
	TickWorld(Scope.World, SettleSeconds);
	const float AfterRelease = GetMana(ASC);

	TickWorld(Scope.World, 2.f);
	TestEqual(TEXT("releasing the channel returns to the vision rate"), AfterRelease - GetMana(ASC), 8.f, Tolerance);

	return true;
}

#endif
