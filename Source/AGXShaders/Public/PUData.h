#pragma once

#include "CoreMinimal.h"
#include "NiagaraCommon.h"
#include "NiagaraTypes.h"
#include "NiagaraDataInterface.h"
#include "NiagaraDataInterfaceRW.h"

class FPUData
{
	struct CoarseParticle
	{
		FVector4 PositionAndRadius = FVector4(0.0, 0.0, 0.0, 0.0);
		FVector4 VelocityAndMass = FVector4(0.0, 0.0, 0.0, 0.0);
	};
	
	static TArray<CoarseParticle> CoarseParticles;
};
