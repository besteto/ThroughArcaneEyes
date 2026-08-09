// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "World/TaeWorldManager.h"
#include "World/TaeRootPath.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

ATaeWorldManager::ATaeWorldManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ATaeWorldManager::BeginPlay()
{
	Super::BeginPlay();

	for (const TObjectPtr<ATaeRootPath>& Path : RootPaths)
	{
		if (Path)
		{
			Path->OnConnectionStateChanged.AddDynamic(this, &ATaeWorldManager::HandleConnectionStateChanged);
		}
	}

	RecountRestored();
	OnNetworkChanged.Broadcast(RestoredCount, GetRequiredCount());
}

void ATaeWorldManager::HandleConnectionStateChanged(ATaeRootPath* Path, ETaeConnectionState NewState)
{
	RecountRestored();
	OnNetworkChanged.Broadcast(RestoredCount, GetRequiredCount());
}

void ATaeWorldManager::RecountRestored()
{
	RestoredCount = 0;
	for (const TObjectPtr<ATaeRootPath>& Path : RootPaths)
	{
		if (Path && Path->GetConnectionState() == ETaeConnectionState::Restored)
		{
			++RestoredCount;
		}
	}
}

#if WITH_EDITOR
EDataValidationResult ATaeWorldManager::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (RootPaths.IsEmpty())
	{
		Context.AddError(FText::FromString(TEXT("RootPaths is empty — assign the ATaeRootPath actors for this level")));
		Result = EDataValidationResult::Invalid;
	}

	TSet<TObjectPtr<ATaeRootPath>> SeenPaths;
	for (int32 Index = 0; Index < RootPaths.Num(); ++Index)
	{
		const TObjectPtr<ATaeRootPath>& Path = RootPaths[Index];
		if (!Path)
		{
			Context.AddError(FText::FromString(FString::Printf(
				TEXT("RootPaths[%d] is null — assign an ATaeRootPath actor or remove this entry"), Index)));
			Result = EDataValidationResult::Invalid;
			continue;
		}

		bool bAlreadyInSet = false;
		SeenPaths.Add(Path, &bAlreadyInSet);
		if (bAlreadyInSet)
		{
			Context.AddError(FText::FromString(FString::Printf(
				TEXT("RootPaths[%d] duplicates an entry already in the array — remove the duplicate"), Index)));
			Result = EDataValidationResult::Invalid;
		}
	}

	return Result;
}
#endif
