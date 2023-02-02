// Copyright 2023, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"

#include "AGX_ShovelEnums.generated.h"

UENUM(BlueprintType)
enum class EAGX_ExcavationMode : uint8
{
	Primary,
	DeformBack,
	DeformRight,
	DeformLeft
};
