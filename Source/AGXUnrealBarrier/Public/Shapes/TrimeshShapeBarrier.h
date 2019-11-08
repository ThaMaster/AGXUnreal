#pragma once

#include "Shapes/ShapeBarrier.h"

#include "RawMesh.h"
#include "Math/Vector.h"

#include <memory>

struct FTriIndices;
class UWorld;

class AGXUNREALBARRIER_API FTrimeshShapeBarrier : public FShapeBarrier
{
public:
	FTrimeshShapeBarrier();
	FTrimeshShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native);
	FTrimeshShapeBarrier(FTrimeshShapeBarrier&& Other);
	virtual ~FTrimeshShapeBarrier() override;

	FRawMesh GetRawMesh(const UWorld* World) const;

	/**
	 * One FVector per vertex location. Vertex positions can be shared between
	 * triangles.
	 */
	TArray<FVector> GetVertexPositions(const UWorld* World) const;

	/**
	 * Mapping from triangles to vertex positions. Each three consecutive
	 * indices for a triangle between the three pointed-to vertex positions.
	 * Several triangles may reference the same vertex position.
	 */
	TArray<uint32> GetVertexIndices() const;

	/**
	 * Per-triangle normal.
	 */
	TArray<FVector> GetTriangleNormals(const UWorld* World) const;

	/**
	 * The source name is a user-provided string that is stored with the trimesh.
	 * it can be the name of a file on disk from which the mesh data was read,
	 * or some other form of description. May be the empty string.
	 * @return The source name that has been stored with the trimesh.
	 */
	FString GetSourceName() const;

	void AllocateNative(
		const TArray<FVector>& Vertices, const TArray<FTriIndices>& TriIndices, bool bClockwise, UWorld* World);

private:
	virtual void AllocateNativeShape() override;
	virtual void ReleaseNativeShape() override;

private:
	FTrimeshShapeBarrier(const FTrimeshShapeBarrier&) = delete;
	void operator=(const FTrimeshShapeBarrier&) = delete;

	struct AllocationParameters
	{
		const TArray<FVector>* Vertices;
		const TArray<FTriIndices>* TriIndices;
		bool bClockwise;
		UWorld* World;
	};

	std::weak_ptr<AllocationParameters> TemporaryAllocationParameters;
};
