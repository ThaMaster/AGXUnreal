#include "GPUHashTableDI.h"
#include "NiagaraTypes.h"
#include "NiagaraRenderer.h"

UGPUHashTableDI::UGPUHashTableDI(FObjectInitializer const& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Optionally Do stuff here!
}

void UGPUHashTableDI::PostInitProperties()
{
	Super::PostInitProperties();

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		ENiagaraTypeRegistryFlags Flags =
			ENiagaraTypeRegistryFlags::AllowAnyVariable | ENiagaraTypeRegistryFlags::AllowParameter;
		FNiagaraTypeRegistry::Register(FNiagaraTypeDefinition(GetClass()), Flags);
	}
}

void UGPUHashTableDI::GetFunctions(TArray<FNiagaraFunctionSignature>& OutFunctions)
{
	FNiagaraFunctionSignature Sig;

	Sig.Name = TEXT("Insert");
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Key")));
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Value")));
	Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("Success")));
	OutFunctions.Add(Sig);

	Sig.Name = TEXT("Lookup");
	Sig.Inputs.Empty();
	Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Key")));
	Sig.Outputs.Empty();
	Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Value")));
	OutFunctions.Add(Sig);
}
