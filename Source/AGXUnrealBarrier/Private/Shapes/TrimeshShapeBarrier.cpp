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
}

FTrimeshShapeBarrier::FTrimeshShapeBarrier()
	: FShapeBarrier()
{
}

FTrimeshShapeBarrier::FTrimeshShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native)
	: FShapeBarrier(std::move(Native))
{
	/// \todo It seems Shape::is<T>() broke with Unreal Engine 2.24. Now we trip
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

TArray<FVector> FTrimeshShapeBarrier::GetVertexPositions() const
{
	TArray<FVector> PositionsUnreal;

	const agxCollide::Trimesh* Trimesh = NativeTrimesh(this, TEXT("fetch positions from"));
	if (Trimesh == nullptr)
	{
		return PositionsUnreal;
	}

	const agx::Vec3Vector& PositionsAgx = Trimesh->getMeshData()->getVertices();
	if (!CheckSize(PositionsAgx.size(), TEXT("vertices")))
	{
		return PositionsUnreal;
	}

	PositionsUnreal.Reserve(static_cast<int32>(PositionsAgx.size()));
	for (const agx::Vec3& Position : PositionsAgx)
	{
		PositionsUnreal.Add(ConvertVector(Position));
	}

	return PositionsUnreal;
}

TArray<uint32> FTrimeshShapeBarrier::GetVertexIndices() const
{
	TArray<uint32> IndicesUnreal;

	const agxCollide::Trimesh* Trimesh = NativeTrimesh(this, TEXT("fetch indices from"));
	if (Trimesh == nullptr)
	{
		return IndicesUnreal;
	}

	const agx::UInt32Vector& IndicesAgx = Trimesh->getMeshData()->getIndices();
	if (!CheckSize(IndicesAgx.size(), TEXT("vertex indices")))
	{
		return IndicesUnreal;
	}

	IndicesUnreal.Reserve(static_cast<int32>(IndicesAgx.size()));
	for (agx::UInt32 Index : IndicesAgx)
	{
		// Assuming agx::UInt32 to uint32 conversion is always safe.
		IndicesUnreal.Add(static_cast<uint32>(Index));
	}

	return IndicesUnreal;
}

TArray<FVector> FTrimeshShapeBarrier::GetTriangleNormals() const
{
	TArray<FVector> NormalsUnreal;

	const agxCollide::Trimesh* Trimesh = NativeTrimesh(this, TEXT("fetch triangle normals from"));
	if (Trimesh == nullptr)
	{
		return NormalsUnreal;
	}

	const agx::Vec3Vector& NormalsAgx = Trimesh->getMeshData()->getNormals();
	if (!CheckSize(NormalsAgx.size(), TEXT("normals")))
	{
		return NormalsUnreal;
	}

	NormalsUnreal.Reserve(static_cast<int32>(NormalsAgx.size()));
	for (const agx::Vec3& Normal : NormalsAgx)
	{
		NormalsUnreal.Add(ConvertVector(Normal));
	}

	return NormalsUnreal;
}

TArray<FVector2D> FTrimeshShapeBarrier::GetRenderDataTextureCoordinates() const
{
	TArray<FVector2D> TextureCoord;

	const agxCollide::RenderData* RenderData =
		GetRenderData(this, TEXT("fetch texture coordinates from"));
	if (RenderData == nullptr)
	{
		return TextureCoord;
	}

	const agx::Vec2Vector& TexCoordArray = RenderData->getTexCoordArray();
	if (!CheckSize(TexCoordArray.size(), TEXT("texture coordinates")))
	{
		return TextureCoord;
	}

	TextureCoord.Reserve(static_cast<int32>(TexCoordArray.size()));
	for (const agx::Vec2& TexCoord : TexCoordArray)
	{
		TextureCoord.Add(Convert(TexCoord));
	}

	return TextureCoord;
}

TArray<uint32> FTrimeshShapeBarrier::GetRenderDataIndices() const
{
	TArray<uint32> VertexIndices;

	const agxCollide::RenderData* RenderData =
		GetRenderData(this, TEXT("fetch render data indices from"));
	if (RenderData == nullptr)
	{
		return VertexIndices;
	}
	const agx::UInt32Vector& IndicesAgx = RenderData->getIndexArray();
	if (!CheckSize(IndicesAgx.size(), TEXT("render data indices")))
	{
		return VertexIndices;
	}
	VertexIndices.Reserve(static_cast<int32>(IndicesAgx.size()));

	for (agx::UInt32 Index : IndicesAgx)
	{
		// Assuming agx::UInt32 to uint32 conversion is always safe.
		VertexIndices.Add(static_cast<uint32>(Index));
	}

	return VertexIndices;
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
