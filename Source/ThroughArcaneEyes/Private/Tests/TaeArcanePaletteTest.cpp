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
