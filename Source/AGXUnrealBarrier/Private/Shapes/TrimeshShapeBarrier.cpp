#include "Shapes/TrimeshShapeBarrier.h"

#include "AGXRefs.h"
#include "AGX_LogCategory.h"

#include "BeginAGXIncludes.h"
#include <agxCollide/Trimesh.h>
#include "EndAGXIncludes.h"

#include "TypeConversions.h"
#include "Interfaces/Interface_CollisionDataProvider.h"

#include "Misc/AssertionMacros.h"

namespace
{
	agxCollide::Trimesh* NativeTrimesh(FTrimeshShapeBarrier* Barrier)
	{
		return Barrier->GetNative()->NativeShape->as<agxCollide::Trimesh>();
	}

	const agxCollide::Trimesh* NativeTrimesh(const FTrimeshShapeBarrier* Barrier)
	{
		return Barrier->GetNative()->NativeShape->as<agxCollide::Trimesh>();
	}

	const agxCollide::Trimesh* NativeTrimesh(
		const FTrimeshShapeBarrier* Barrier, const TCHAR* Operation)
	{
		if (!Barrier->HasNative())
		{
			UE_LOG(
				LogAGX, Warning, TEXT("Cannot %s Trimesh barrier without a native Trimesh"),
				Operation);
			return nullptr;
		}

		const agxCollide::Trimesh* Native = NativeTrimesh(Barrier);
		if (Native == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Cannot %s Trimesh barrier whose native shape is not a Trimesh."));
		}

		return Native;
	}

	agxCollide::Trimesh* NativeTrimesh(FTrimeshShapeBarrier* Barrier, const TCHAR* Operation)
	{
		const FTrimeshShapeBarrier* ConstBarrier = const_cast<const FTrimeshShapeBarrier*>(Barrier);
		const agxCollide::Trimesh* ConstTrimesh = NativeTrimesh(ConstBarrier, Operation);
		agxCollide::Trimesh* Trimesh = const_cast<agxCollide::Trimesh*>(ConstTrimesh);
		return Trimesh;
	}

	const agxCollide::RenderData* GetRenderData(
		const FTrimeshShapeBarrier* Barrier, const TCHAR* Operation)
	{
		const agxCollide::Trimesh* Native = NativeTrimesh(Barrier, Operation);
		if (Native == nullptr)
		{
			return nullptr;
		}

		const agxCollide::RenderData* RenderData = Native->getRenderData();
		if (RenderData == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Cannot %s Trimesh barrier whose native Trimesh doesn't contain render data."),
				Operation);
			return nullptr;
		}

		return RenderData;
	}

	agxCollide::RenderData* GetRenderData(FTrimeshShapeBarrier* Barrier, const TCHAR* Operation)
	{
		const FTrimeshShapeBarrier* ConstBarrier = const_cast<const FTrimeshShapeBarrier*>(Barrier);
		const agxCollide::RenderData* ConstRenderData = GetRenderData(ConstBarrier, Operation);
		agxCollide::RenderData* RenderData = const_cast<agxCollide::RenderData*>(ConstRenderData);
		return RenderData;
	}

	bool CheckSize(size_t Size)
	{
		return Size <= static_cast<size_t>(std::numeric_limits<int32>::max());
	}

	bool CheckSize(size_t Size, const TCHAR* DataName)
	{
		bool Ok = CheckSize(Size);
		if (!Ok)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Native trimesh contains more %s than Unreal can handle. Size is %zu."),
				DataName, Size);
		}
		return Ok;
	}

	template <typename AgxType, typename UnrealType, typename FGetAgxBuffer, typename FConvert>
	TArray<UnrealType> ConvertCollisionBuffer(
		const FTrimeshShapeBarrier* Barrier, const TCHAR* Operation, const TCHAR* DataName,
		FGetAgxBuffer GetAgxBuffer, FConvert Convert)
	{
		TArray<UnrealType> DataUnreal;
		const agxCollide::Trimesh* Trimesh = NativeTrimesh(Barrier, Operation);
		if (Trimesh == nullptr)
		{
			return DataUnreal;
		}
		const agx::VectorPOD<AgxType>& DataAgx = GetAgxBuffer(Trimesh->getMeshData());
		if (!CheckSize(DataAgx.size(), DataName))
		{
			return DataUnreal;
		}
		DataUnreal.Reserve(static_cast<int32>(DataAgx.size()));
		for (AgxType DatumAgx : DataAgx)
		{
			DataUnreal.Add(Convert(DatumAgx));
		}
		return DataUnreal;
	}

	template <typename AgxType, typename UnrealType, typename FGetAgxBuffer, typename FConvert>
	TArray<UnrealType> ConvertRenderBuffer(
		const FTrimeshShapeBarrier* Barrier, const TCHAR* Operation, const TCHAR* DataName,
		FGetAgxBuffer GetAgxBuffer, FConvert Convert)
	{
		TArray<UnrealType> DataUnreal;
		const agxCollide::RenderData* RenderData = GetRenderData(Barrier, Operation);
		if (RenderData == nullptr)
		{
			return DataUnreal;
		}
		const agx::VectorPOD<AgxType>& DataAgx = GetAgxBuffer(RenderData);
		if (!CheckSize(DataAgx.size(), DataName))
		{
			return DataUnreal;
		}
		DataUnreal.Reserve(static_cast<int32>(DataAgx.size()));
		for (AgxType DatumAgx : DataAgx)
		{
			DataUnreal.Add(Convert(DatumAgx));
		}
		return DataUnreal;
	}
}

FTrimeshShapeBarrier::FTrimeshShapeBarrier()
	: FShapeBarrier()
{
}

FTrimeshShapeBarrier::FTrimeshShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native)
	: FShapeBarrier(std::move(Native))
{
	/// \todo It seems Shape::is<T>() broken with Unreal Engine 2.24. Now we trip
	/// on this check when restoring AGX Dynamics archives containing
	/// trimeshes. I suspect it's at least partially related to
	///     #define dynamic_cast UE4Casts_Private::DynamicCast
	/// in Casts.h
	// check(NativeRef->NativeShape->is<agxCollide::Trimesh>());
	check(NativeRef->NativeShape->getType() == agxCollide::Shape::TRIMESH);
}

FTrimeshShapeBarrier::FTrimeshShapeBarrier(FTrimeshShapeBarrier&& Other)
	: FShapeBarrier(std::move(Other))
{
}

FTrimeshShapeBarrier::~FTrimeshShapeBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FTrimeshShapeRef.
}

int32 FTrimeshShapeBarrier::GetNumPositions() const
{
	const agxCollide::Trimesh* Trimesh = NativeTrimesh(this, TEXT("fetch num positions from"));
	if (Trimesh == nullptr)
	{
		return -1;
	}
	if (!CheckSize(Trimesh->getNumVertices(), TEXT("positions")))
	{
		return -1;
	}
	return static_cast<int>(Trimesh->getNumVertices());
}

int32 FTrimeshShapeBarrier::GetNumIndices() const
{
	const agxCollide::Trimesh* Trimesh = NativeTrimesh(this, TEXT("fetch num indices"));
	if (Trimesh == nullptr)
	{
		return -1;
	}
	const size_t NumIndices = Trimesh->getMeshData()->getIndices().size();
	if (NumIndices % 3 != 0)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Trimesh '%s' has invalid collision data. The number of vertex indices isn't a "
				 "multiple of 3. The last triangle will be skipped."),
			*GetSourceName())
	}
	if (!CheckSize(NumIndices, TEXT("indices")))
	{
		return -1;
	}
	return static_cast<int>(NumIndices);
}

int32 FTrimeshShapeBarrier::GetNumTriangles() const
{
	const agxCollide::Trimesh* Trimesh = NativeTrimesh(this, TEXT("fetch num triangles"));
	if (Trimesh == nullptr)
	{
		return -1;
	}
	const size_t NumTriangles = Trimesh->getNumTriangles();
	if (!CheckSize(NumTriangles, TEXT("triangles")))
	{
		return -1;
	}
	return static_cast<int>(NumTriangles);
}

TArray<FVector> FTrimeshShapeBarrier::GetVertexPositions() const
{
	return ConvertCollisionBuffer<agx::Vec3, FVector>(
		this, TEXT("fetch positions from"), TEXT("positions"),
		[](const agxCollide::MeshData* Mesh) -> auto& { return Mesh->getVertices(); },
		[](const agx::Vec3& Position) { return ConvertVector(Position); });
}

TArray<uint32> FTrimeshShapeBarrier::GetVertexIndices() const
{
	return ConvertCollisionBuffer<agx::UInt32, uint32>(
		this, TEXT("fetch indices from"), TEXT("vertex indices"),
		[](const agxCollide::MeshData* Mesh) -> auto& { return Mesh->getIndices(); },
		[](const agx::UInt32 Index) { return static_cast<uint32>(Index); });
}

TArray<FVector> FTrimeshShapeBarrier::GetTriangleNormals() const
{
	return ConvertCollisionBuffer<agx::Vec3, FVector>(
		this, TEXT("fetch triangle normals from"), TEXT("normals"),
		[](const agxCollide::CollisionMeshData* Mesh) -> auto& { return Mesh->getNormals(); },
		[](const agx::Vec3& Normal) { return ConvertVector(Normal); });
}

int32 FTrimeshShapeBarrier::GetNumRenderPositions() const
{
	const agxCollide::RenderData* RenderData =
		GetRenderData(this, TEXT("fetch num render positions"));
	if (RenderData == nullptr)
	{
		return -1;
	}
	const size_t NumPositions = RenderData->getVertexArray().size();
	if (!CheckSize(NumPositions, TEXT("render positions")))
	{
		return -1;
	}
	return static_cast<int32>(NumPositions);
}

int32 FTrimeshShapeBarrier::GetNumRenderTriangles() const
{
	const agxCollide::RenderData* RenderData = GetRenderData(this, TEXT("fetch num triangles"));
	if (RenderData == nullptr)
	{
		return -1;
	}
	const size_t NumIndices = RenderData->getIndexArray().size();
	if (NumIndices % 3 != 0)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Trimesh '%s' has invalid render data. The number of vertex indices isn't a "
				 "multiple of 3. The last triangle will be skipped."),
			*GetSourceName())
	}
	const size_t NumTriangles = NumIndices / 3;
	if (!CheckSize(NumTriangles, TEXT("render triangles")))
	{
		return -1;
	}
	return static_cast<int32>(NumTriangles);
}

int32 FTrimeshShapeBarrier::GetNumRenderIndices() const
{
	const agxCollide::RenderData* RenderData =
		GetRenderData(this, TEXT("fetch num render indices"));
	if (RenderData == nullptr)
	{
		return -1;
	}
	const size_t NumIndices = RenderData->getIndexArray().size();
	if (!CheckSize(NumIndices, TEXT("render indices")))
	{
		return -1;
	}
	return static_cast<int32>(NumIndices);
}

TArray<uint32> FTrimeshShapeBarrier::GetRenderDataIndices() const
{
	return ConvertRenderBuffer<agx::UInt32, uint32>(
		this, TEXT("fetch render data indices from"), TEXT("render data indices"),
		[](const agxCollide::RenderData* Render) -> auto& { return Render->getIndexArray(); },
		[](const agx::UInt32 Index) { return static_cast<uint32>(Index); });
}

TArray<FVector> FTrimeshShapeBarrier::GetRenderDataPositions() const
{
	return ConvertRenderBuffer<agx::Vec3, FVector>(
		this, TEXT("fetch render data positions from"), TEXT("render data positions"),
		[](const agxCollide::RenderData* Render) -> auto& { return Render->getVertexArray(); },
		[](const agx::Vec3& Position) { return ConvertVector(Position); });
}

TArray<FVector> FTrimeshShapeBarrier::GetRenderDataNormals() const
{
	return ConvertRenderBuffer<agx::Vec3, FVector>(
		this, TEXT("fetch render data normals"), TEXT("render data normals"),
		[](const agxCollide::RenderData* Render) -> auto& { return Render->getNormalArray(); },
		[](const agx::Vec3& Normal) { return ConvertVector(Normal); });
}

TArray<FVector2D> FTrimeshShapeBarrier::GetRenderDataTextureCoordinates() const
{
	return ConvertRenderBuffer<agx::Vec2, FVector2D>(
		this, TEXT("fetch texture coordinates from"), TEXT("texture coordinates"),
		[](const agxCollide::RenderData* Render) -> auto& { return Render->getTexCoordArray(); },
		[](const agx::Vec2& Coordinate) { return Convert(Coordinate); });
}

FString FTrimeshShapeBarrier::GetSourceName() const
{
	FString SourceName;
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Cannot fetch triangle soure name from Trimesh barrier without a native Trimesh"));
		return SourceName;
	}

	const agxCollide::Trimesh* Trimesh = NativeTrimesh(this);
	SourceName = Convert(Trimesh->getSourceName());
	return SourceName;
}

FGuid FTrimeshShapeBarrier::GetMeshDataGuid() const
{
	check(HasNative());
	if (const agxCollide::Trimesh* Trimesh = NativeTrimesh(this, TEXT("fetch mesh data guid")))
	{
		return Convert(Trimesh->getMeshData()->getUuid());
	}
	return FGuid();
}

void FTrimeshShapeBarrier::AllocateNative(
	const TArray<FVector>& Vertices, const TArray<FTriIndices>& TriIndices, bool bClockwise,
	const FString& SourceName)
{
	{
		// Create temporary allocation parameters structure for AllocateNativeShape() to use.

		std::shared_ptr<AllocationParameters> Params =
			std::make_shared<AllocationParameters>(SourceName);
		Params->Vertices = &Vertices;
		Params->TriIndices = &TriIndices;
		Params->bClockwise = bClockwise;

		TemporaryAllocationParameters = Params;

		FShapeBarrier::AllocateNative(); // Will implicitly invoke AllocateNativeShape(). See below.
	}
	// Temporary allocation parameters structure destroyed by smart pointer.
}

void FTrimeshShapeBarrier::AllocateNativeShape()
{
	check(!HasNative());

	// Retrieve the temporary allocation parameters.

	std::shared_ptr<AllocationParameters> Params = TemporaryAllocationParameters.lock();
	check(Params != nullptr);

	// Transfer to native buffers.

	agx::Vec3Vector NativeVertices;
	NativeVertices.reserve(Params->Vertices->Num());

	for (const FVector& Vertex : *Params->Vertices)
	{
		NativeVertices.push_back(ConvertVector(Vertex));
	}

	agx::UInt32Vector NativeIndices;
	NativeIndices.reserve(Params->TriIndices->Num() * 3);

	for (const FTriIndices& Index : *Params->TriIndices)
	{
		check(Index.v0 >= 0);
		check(Index.v1 >= 0);
		check(Index.v2 >= 0);

		NativeIndices.push_back(static_cast<uint32>(Index.v0));
		NativeIndices.push_back(static_cast<uint32>(Index.v1));
		NativeIndices.push_back(static_cast<uint32>(Index.v2));
	}

	agxCollide::Trimesh::TrimeshOptionsFlags OptionsMask =
		Params->bClockwise ? agxCollide::Trimesh::TrimeshOptionsFlags::CLOCKWISE_ORIENTATION
						   : static_cast<agxCollide::Trimesh::TrimeshOptionsFlags>(0);

	// Create the native object.

	NativeRef->NativeShape = new agxCollide::Trimesh(
		&NativeVertices, &NativeIndices, Convert(Params->SourceName).c_str(), OptionsMask);
}

void FTrimeshShapeBarrier::ReleaseNativeShape()
{
	check(HasNative());
	NativeRef->NativeShape = nullptr;
}
