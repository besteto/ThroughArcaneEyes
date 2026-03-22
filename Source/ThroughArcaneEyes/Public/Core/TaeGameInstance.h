// Copyright © 2026 Helen Allien Poe. Source available — see LICENSE.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TaeGameInstance.generated.h"

UCLASS()
class THROUGHARCANEEYES_API UTaeGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;
};
