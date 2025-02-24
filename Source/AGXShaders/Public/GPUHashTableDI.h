#pragma once

#include "CoreMinimal.h"
#include "NiagaraCommon.h"
#include "NiagaraTypes.h"
#include "NiagaraDataInterface.h"
#include "GPUHashTableDI.generated.h"


UCLASS(EditInlineNew, Category = "Niagara", CollapseCategories, meta = (DisplayName = "GPU Hash Table"))
class AGXSHADERS_API UGPUHashTableDI : public UNiagaraDataInterface
{
	GENERATED_UCLASS_BODY()

public:
	// Registers the interface so it becoems available in editor
	virtual void PostInitProperties() override;

	// Creates the functions that are available in the editor
	virtual void GetFunctions(TArray<FNiagaraFunctionSignature>& OutFunctions) override;
	virtual void GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction& OutFunc) override;

	virtual void VMLookupRoom(FVectorVMExternalFunctionContext& Context);

private:
	static const FName InsertIndex;
	static const FName DeleteIndex;
	static const FName AddRoomAtIndex;
	static const FName ClearTable;
	static const FName LookupRoom;
	static const FName LookupPosition;
	static const FName LookupVelocity;
	static const FName LookupBounds;
};
