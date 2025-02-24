#pragma once

#include "CoreMinimal.h"
#include "NiagaraCommon.h"
#include "NiagaraShared.h"
#include "NiagaraTypes.h"
#include "NiagaraDataInterface.h"
#include "NiagaraDataInterfaceRW.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "AGX_Real.h"
#include "GPUHashTableDI.generated.h"

UCLASS(EditInlineNew, Category = "Niagara", CollapseCategories, meta = (DisplayName = "GPU Hash Table"))
class AGXSHADERS_API UGPUHashTableDI : public UNiagaraDataInterface
{
	GENERATED_UCLASS_BODY()

public:

	void PostInitProperties();
	virtual void GetFunctions(TArray<FNiagaraFunctionSignature>& OutFunctions) override;
	//virtual bool InitPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance) override;
	//virtual void DestroyPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance) override;
};
