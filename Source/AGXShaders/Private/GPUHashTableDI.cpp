#include "GPUHashTableDI.h"
#include "NiagaraTypes.h"
#include "NiagaraShaderParametersBuilder.h"

// Names for the data interface functions
const FName UGPUHashTableDI::InsertIndex("InsertIndex");
const FName UGPUHashTableDI::DeleteIndex("InsertIndex");
const FName UGPUHashTableDI::AddRoomAtIndex("AddRoomAtIndex");
const FName UGPUHashTableDI::ClearTable("ClearTable");
const FName UGPUHashTableDI::LookupRoom("LookupRoom");
const FName UGPUHashTableDI::LookupPosition("LookupPosition");
const FName UGPUHashTableDI::LookupVelocity("LookupVelocity");
const FName UGPUHashTableDI::LookupBounds("LookupBounds");

UGPUHashTableDI::UGPUHashTableDI(FObjectInitializer const& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Initialize the hash table here!
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
	// ---------- WRITE FUNCTIONS ---------- //
	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = InsertIndex;
		Sig.Inputs.Empty();
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("DataGrid")));
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Index")));
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Room")));
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Position")));
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Velocity")));
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("MinBounds")));
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("MaxBounds")));
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("InnerMinBounds")));
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("InnerMaxBounds")));
		Sig.Outputs.Empty();
		Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("Success")));

		Sig.bMemberFunction = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		Sig.bWriteFunction = true;
		Sig.bRequiresExecPin = true;
		Sig.Description = NSLOCTEXT("Niagara", "GPUHashTableDI_InsertIndex", "Inserts the specified data for the voxel with the specified index.");
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = DeleteIndex;
		Sig.Inputs.Empty();
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("DataGrid")));
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Index")));
		Sig.Outputs.Empty();
		Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("Success")));

		Sig.bMemberFunction = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		Sig.bWriteFunction = true;
		Sig.bRequiresExecPin = true;
		Sig.Description = NSLOCTEXT("Niagara", "GPUHashTableDI_DeleteIndex", "Clears the voxel with the specified index.");
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = AddRoomAtIndex;
		Sig.Inputs.Empty();
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("DataGrid")));
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Index")));
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Amount")));
		Sig.Outputs.Empty();
		Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Result")));
		Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("Success")));

		Sig.bMemberFunction = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		Sig.bWriteFunction = true;
		Sig.Description = NSLOCTEXT("Niagara", "GPUHashTableDI_AddRoomAtIndex", "Adds room at a specified index. NOTE: Use this to remove room too.");
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = ClearTable;
		Sig.Inputs.Empty();
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("DataGrid")));
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Id")));
		Sig.Outputs.Empty();
		Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("Success")));

		Sig.bMemberFunction = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		Sig.bWriteFunction = true;
		Sig.bRequiresExecPin = true;
		Sig.Description = NSLOCTEXT("Niagara", "GPUHashTableDI_ClearTable", "Clears the hashtable. Id is the thread id.");
		OutFunctions.Add(Sig);
	}

	// ---------- READ FUNCTIONS ---------- //

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = LookupRoom;
		Sig.Inputs.Empty();
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("DataGrid")));
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Index")));
		Sig.Outputs.Empty();
		Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Room")));
		Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("Success")));

		Sig.bMemberFunction = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		Sig.bReadFunction = true;
		Sig.bRequiresExecPin = false;
		Sig.Description = NSLOCTEXT("Niagara", "GPUHashTableDI_LookupRoom", "Gets the amount of room in the voxel at the specified index");
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = LookupPosition;
		Sig.Inputs.Empty();
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("DataGrid")));
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Index")));
		Sig.Outputs.Empty();
		Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Position")));
		Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("Success")));

		Sig.bMemberFunction = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		Sig.bReadFunction = true;
		Sig.bRequiresExecPin = false;
		Sig.Description = NSLOCTEXT("Niagara", "GPUHashTableDI_LookupPosition", "Gets the position of the voxel at the specified index");
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name =LookupVelocity;
		Sig.Inputs.Empty();
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("DataGrid")));
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Index")));
		Sig.Outputs.Empty();
		Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Velocity")));
		Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("Success")));

		Sig.bMemberFunction = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		Sig.bReadFunction = true;
		Sig.bRequiresExecPin = false;
		Sig.Description = NSLOCTEXT("Niagara", "GPUHashTableDI_LookupVelocity", "Gets the velocity of the voxel at the specified index.");
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = LookupBounds;
		Sig.Inputs.Empty();
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("DataGrid")));
		Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("Index")));
		Sig.Outputs.Empty();
		Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("MinBounds")));
		Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("MaxBounds")));
		Sig.Outputs.Add(
			FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("InnerMinBounds")));
		Sig.Outputs.Add(
			FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("InnerMaxBounds")));
		Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("Success")));

		Sig.bMemberFunction = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		Sig.bReadFunction = true;
		Sig.bRequiresExecPin = false;
		Sig.Description = NSLOCTEXT("Niagara", "GPUHashTableDI_LookupBounds", "Gets the bounds of the voxel at the specified index");
		OutFunctions.Add(Sig);
	}
}

void UGPUHashTableDI::GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction& OutFunc)
{
	if (BindingInfo.Name == InsertIndex)
	{

	}
	else if (BindingInfo.Name == DeleteIndex)
	{

	}
	else if (BindingInfo.Name == AddRoomAtIndex)
	{

	}
	else if (BindingInfo.Name == ClearTable)
	{

	}
	else if (BindingInfo.Name == LookupRoom)
	{

	}
	else if (BindingInfo.Name == LookupPosition)
	{

	}
	else if (BindingInfo.Name == LookupVelocity)
	{

	}
	else if (BindingInfo.Name == LookupBounds)
	{

	}
}

void UGPUHashTableDI::VMLookupRoom(FVectorVMExternalFunctionContext& Context)
{

}
