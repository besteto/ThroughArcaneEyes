// Copyright © 2026 Helen Allien Poe. See LICENSE.

#include "World/TaeConnectionTypes.h"

float FTaeGrowthStep::Advance(const float CurrentAlpha, const float DeltaAlpha)
{
	return FMath::Clamp(CurrentAlpha + DeltaAlpha, 0.f, 1.f);
}

ETaeConnectionState FTaeGrowthStep::StateFor(const float Alpha)
{
	if (Alpha >= 1.f)
	{
		return ETaeConnectionState::Restored;
	}
	if (Alpha > 0.f)
	{
		return ETaeConnectionState::Growing;
	}
	return ETaeConnectionState::Broken;
}
