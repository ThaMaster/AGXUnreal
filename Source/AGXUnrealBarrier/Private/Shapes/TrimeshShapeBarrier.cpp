#include "Shapes/TrimeshShapeBarrier.h"

#include "AGXRefs.h"

#include "BeginAGXIncludes.h"
#include <agxCollide/Trimesh.h>
#include "EndAGXIncludes.h"

#include "TypeConversions.h"

#include "Misc/AssertionMacros.h"

#include "Engine/World.h"

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
	check(NativeRef->NativeShape->is<agxCollide::Trimesh>());
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

FRawMesh FTrimeshShapeBarrier::GetRawMesh(const UWorld* World) const
{
	FRawMesh RawMesh;

	RawMesh.VertexPositions = GetVertexPositions(World);
	RawMesh.WedgeIndices = GetVertexIndices();
	TArray<FVector> TriangleNormals = GetTriangleNormals(World);

	const int32 NumFaces = TriangleNormals.Num();
	const int32 NumWedges = RawMesh.WedgeIndices.Num();
	check(NumWedges == 3 * NumFaces);

	RawMesh.FaceMaterialIndices.Reserve(NumFaces);
	RawMesh.FaceSmoothingMasks.Reserve(NumFaces);
	for (int32 FaceIndex = 0; FaceIndex < NumFaces; ++FaceIndex)
	{
		RawMesh.FaceMaterialIndices.Add(0);
		RawMesh.FaceSmoothingMasks.Add(0xFFFFFFFF);
	}

	RawMesh.WedgeTangentX.Reserve(NumWedges);
	RawMesh.WedgeTangentY.Reserve(NumWedges);
	RawMesh.WedgeTangentZ.Reserve(NumWedges);
	RawMesh.WedgeColors.Reserve(NumWedges);
	for (int32 UVIndex = 0; UVIndex < MAX_MESH_TEXTURE_COORDS; ++UVIndex)
	{
		RawMesh.WedgeTexCoords[UVIndex].Reserve(NumWedges);
	}

	for (int32 i = 0; i < NumWedges; ++i)
	{
		RawMesh.WedgeTangentX.Add(FVector(0.0f, 0.0f, 0.0f));
		RawMesh.WedgeTangentY.Add(FVector(0.0f, 0.0f, 0.0f));
		RawMesh.WedgeColors.Add(FColor(255, 255, 255));
		for (int32 UVIndex = 0; UVIndex < MAX_MESH_TEXTURE_COORDS; ++UVIndex)
		{
			RawMesh.WedgeTexCoords[UVIndex].Add(FVector2D(0.0f, 0.0f));
		}
	}

	for (const FVector& TriangleNormal : TriangleNormals)
	{
		// The native trimesh store normals per triangle and not per vertex as
		// Unreal want it. Duplicating the normals here, but if we want smooth
		// shading then we should try to let Unreal compute the normals for us.
		RawMesh.WedgeTangentZ.Add(TriangleNormal);
		RawMesh.WedgeTangentZ.Add(TriangleNormal);
		RawMesh.WedgeTangentZ.Add(TriangleNormal);
	}

	return RawMesh;
}

TArray<FVector> FTrimeshShapeBarrier::GetVertexPositions(const UWorld* World) const
{
	TArray<FVector> VertexPositions;

	if (!HasNative())
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot fetch positions from Trimesh barrier without a native Trimesh."));
		return VertexPositions;
	}

	const agxCollide::Trimesh* Trimesh = NativeTrimesh(this);
	if (Trimesh == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Native shape is not a Trimesh."));
		return VertexPositions;
	}

	size_t NumVerticesAGX = Trimesh->getNumVertices();
	if (NumVerticesAGX > static_cast<size_t>(std::numeric_limits<int32>::max()))
	{
		UE_LOG(LogTemp, Error, TEXT("Native trimesh contains more vertices than Unreal can handle."));
		return VertexPositions;
	}

	int32 NumVertices = static_cast<int32>(NumVerticesAGX);
	VertexPositions.Reserve(NumVertices);

	const agxCollide::CollisionMeshData* MeshData = Trimesh->getMeshData();
	const agx::Vec3Vector& Positions = MeshData->getVertices();
	for (const agx::Vec3& Position : Positions)
	{
		VertexPositions.Add(ConvertVector(Position, World));
	}

	return VertexPositions;
}

TArray<uint32> FTrimeshShapeBarrier::GetVertexIndices() const
{
	TArray<uint32> VertexIndices;

	if (!HasNative())
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot fetch vertex indices from Trimesh barrier without a native Trimesh."));
		return VertexIndices;
	}

	const agxCollide::Trimesh* Trimesh = NativeTrimesh(this);
	if (Trimesh == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Native shape is not a Trimesh."));
		return VertexIndices;
	}

	size_t NumTriangles = Trimesh->getNumTriangles();
	if (NumTriangles * 3 > static_cast<size_t>(std::numeric_limits<int32>::max()))
	{
		UE_LOG(LogTemp, Error, TEXT("Native trimesh contains more triangles than Unreal can handle"));
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

TArray<FVector> FTrimeshShapeBarrier::GetTriangleNormals(const UWorld* World) const
{
	TArray<FVector> TriangleNormals;

	if (!HasNative())
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot fetch triangle normals from Trimesh barrier without a native Trimesh."));
		return TriangleNormals;
	}

	const agxCollide::Trimesh* Trimesh = NativeTrimesh(this);
	if (Trimesh == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("NativeShape is not a Trimesh."));
		return TriangleNormals;
	}

	const size_t NumTrianglesAGX = Trimesh->getNumTriangles();
	if (NumTrianglesAGX > static_cast<size_t>(std::numeric_limits<int32>::max()))
	{
		UE_LOG(LogTemp, Error, TEXT("Native trimesh contains more triangles than Unreal can handle"));
		return TriangleNormals;
	}

	const int32 NumTriangles = static_cast<int32>(NumTrianglesAGX);
	TriangleNormals.Reserve(NumTriangles);
	const agx::Vec3Vector& Normals = Trimesh->getMeshData()->getNormals();
	for (const agx::Vec3& Normal : Normals)
	{
		TriangleNormals.Add(ConvertVector(Normal, World));
	}

	return TriangleNormals;
}

FString FTrimeshShapeBarrier::GetSourceName() const
{
	FString SourceName;
	if (!HasNative())
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot fetch triangle soure name from Trimesh barrier without a native Trimesh"));
		return SourceName;
	}

	const agxCollide::Trimesh* Trimesh = NativeTrimesh(this);
	SourceName = Convert(Trimesh->getSourceName());
	return SourceName;
}

void FTrimeshShapeBarrier::AllocateNative(
	const TArray<FVector>& Vertices, const TArray<FTriIndices>& TriIndices, bool bClockwise, UWorld* World)
{
	{
		// Create temporary allocation parameters structure for AllocateNativeShape() to use.

		std::shared_ptr<AllocationParameters> Params = std::make_shared<AllocationParameters>();
		Params->Vertices = &Vertices;
		Params->TriIndices = &TriIndices;
		Params->bClockwise = bClockwise;
		Params->World = World;

		TemporaryAllocationParameters = Params;

		FShapeBarrier::AllocateNative();	// Will implicitly invoke AllocateNativeShape(). See below.
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
		NativeVertices.push_back(ConvertVector(Vertex, Params->World));
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

	NativeRef->NativeShape =
		new agxCollide::Trimesh(&NativeVertices, &NativeIndices, "FTrimeshShapeBarrier", OptionsMask);
}

void FTrimeshShapeBarrier::ReleaseNativeShape()
{
	check(HasNative());
	NativeRef->NativeShape = nullptr;
}
