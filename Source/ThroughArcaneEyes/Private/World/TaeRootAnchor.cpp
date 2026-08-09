// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#include "World/TaeRootAnchor.h"
#include "World/TaeRootPath.h"
#include "Components/SphereComponent.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

ATaeRootAnchor::ATaeRootAnchor()
{
	PrimaryActorTick.bCanEverTick = false;

	ChannelRange = CreateDefaultSubobject<USphereComponent>(TEXT("ChannelRange"));
	SetRootComponent(ChannelRange);
	ChannelRange->SetSphereRadius(200.f);
	ChannelRange->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ChannelRange->SetCollisionResponseToAllChannels(ECR_Overlap);
}

#if WITH_EDITOR
EDataValidationResult ATaeRootAnchor::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (!Path)
	{
		Context.AddError(FText::FromString(TEXT("Path is not set — assign the ATaeRootPath this anchor grows")));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif
