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
	TArray<FVector> VertexPositions;

	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot fetch positions from Trimesh barrier without a native Trimesh."));
		return VertexPositions;
	}

	const agxCollide::Trimesh* Trimesh = NativeTrimesh(this);
	if (Trimesh == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot fetch positions from Trimesh barrier whose native shape is not a "
				 "Trimesh."));
		return VertexPositions;
	}

	size_t NumVerticesAGX = Trimesh->getNumVertices();
	if (NumVerticesAGX > static_cast<size_t>(std::numeric_limits<int32>::max()))
	{
		UE_LOG(
			LogAGX, Error, TEXT("Native trimesh contains more vertices than Unreal can handle."));
		return VertexPositions;
	}

	int32 NumVertices = static_cast<int32>(NumVerticesAGX);
	VertexPositions.Reserve(NumVertices);

	const agxCollide::CollisionMeshData* MeshData = Trimesh->getMeshData();
	const agx::Vec3Vector& Positions = MeshData->getVertices();
	for (const agx::Vec3& Position : Positions)
	{
		VertexPositions.Add(ConvertVector(Position));
	}

	return VertexPositions;
}

TArray<uint32> FTrimeshShapeBarrier::GetVertexIndices() const
{
	TArray<uint32> VertexIndices;

	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot fetch vertex indices from Trimesh barrier without a native Trimesh."));
		return VertexIndices;
	}

	const agxCollide::Trimesh* Trimesh = NativeTrimesh(this);
	if (Trimesh == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot fetch vertex indices from Trimesh barrier whose native shape is not a "
				 "Trimesh."));
		return VertexIndices;
	}

	size_t NumTriangles = Trimesh->getNumTriangles();
	if (NumTriangles * 3 > static_cast<size_t>(std::numeric_limits<int32>::max()))
	{
		UE_LOG(
			LogAGX, Error, TEXT("Native trimesh contains more triangles than Unreal can handle"));
		return VertexIndices;
	}

	const int32 NumIndices = static_cast<int32>(3 * NumTriangles);
	VertexIndices.Reserve(NumIndices);
	const agxCollide::CollisionMeshData* MeshData = Trimesh->getMeshData();
	const agx::UInt32Vector& Indices = MeshData->getIndices();
	for (agx::UInt32 Index : Indices)
	{
		// Assuming agx::UInt32 to uint32 conversion is always safe.
		VertexIndices.Add(static_cast<uint32>(Index));
	}

	return VertexIndices;
}

TArray<FVector> FTrimeshShapeBarrier::GetTriangleNormals() const
{
	TArray<FVector> TriangleNormals;

	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot fetch triangle normals from Trimesh barrier without a native Trimesh."));
		return TriangleNormals;
	}

	const agxCollide::Trimesh* Trimesh = NativeTrimesh(this);
	if (Trimesh == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot fetch triangle normals from Trimesh barrier whose native shape is not a "
				 "Trimesh."));
		return TriangleNormals;
	}

	const size_t NumTrianglesAGX = Trimesh->getNumTriangles();
	if (NumTrianglesAGX > static_cast<size_t>(std::numeric_limits<int32>::max()))
	{
		UE_LOG(
			LogAGX, Error, TEXT("Native trimesh contains more triangles than Unreal can handle"));
		return TriangleNormals;
	}

	const int32 NumTriangles = static_cast<int32>(NumTrianglesAGX);
	TriangleNormals.Reserve(NumTriangles);
	const agx::Vec3Vector& Normals = Trimesh->getMeshData()->getNormals();
	for (const agx::Vec3& Normal : Normals)
	{
		TriangleNormals.Add(ConvertVector(Normal));
	}

	return TriangleNormals;
}

TArray<FVector2D> FTrimeshShapeBarrier::GetRenderDataTextureCoordinates() const
{
	TArray<FVector2D> TextureCoord;

	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"Cannot fetch texture coordinates from Trimesh barrier without a native Trimesh."));
		return TextureCoord;
	}

	const agxCollide::Trimesh* Trimesh = NativeTrimesh(this);
	if (Trimesh == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot fetch texture coordinates from Trimesh barrier whose native shape is not "
				 "a Trimesh."));
		return TextureCoord;
	}

	const agxCollide::RenderData* RenderData = Trimesh->getRenderData();
	if (RenderData == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot fetch texture coordinates from Trimesh barrier whose native Trimesh "
				 "doesn't contain render data."));
		return TextureCoord;
	}

	const agx::Vec2Vector& TexCoordArray = RenderData->getTexCoordArray();
	const size_t NumTexCoord = TexCoordArray.size();
	if (NumTexCoord > static_cast<size_t>(std::numeric_limits<int32>::max()))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Native trimesh's RenderData contains more triangles than Unreal can handle."));
		return TextureCoord;
	}

	TextureCoord.Reserve(NumTexCoord);
	for (const agx::Vec2& TexCoord : TexCoordArray)
	{
		TextureCoord.Add(Convert(TexCoord));
	}

	return TextureCoord;
}

TArray<uint32> FTrimeshShapeBarrier::GetRenderDataVertexIndices() const
{
	TArray<uint32> VertexIndices;

	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot fetch render data vertex indices from Trimesh barrier without a native "
				 "Trimesh."));
		return VertexIndices;
	}

	const agxCollide::Trimesh* Trimesh = NativeTrimesh(this);
	if (Trimesh == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot fetch render data vertex indices from Trimesh barrier whose native shape "
				 "is not a Trimesh."));
		return VertexIndices;
	}

	const agxCollide::RenderData* RenderData = Trimesh->getRenderData();
	if (RenderData == nullptr)
	{
		UE_LOG(LogAGX, Error, TEXT("Cannot fetch render data vertex indices from Trimesh barrier whose native Trimesh doesn't contain render data."));
		return VertexIndices;
	}

	const agx::UInt32Vector& Indices = RenderData->getIndexArray();
	const size_t NumIndices = Indices.size();
	if (NumIndices > static_cast<size_t>(std::numeric_limits<int32>::max()))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Native trimesh's RenderData contains more triangles than Unreal can handle."));
		return VertexIndices;
	}

	for (agx::UInt32 Index : Indices)
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
