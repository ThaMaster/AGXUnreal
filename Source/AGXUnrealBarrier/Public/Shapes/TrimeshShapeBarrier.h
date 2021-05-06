#pragma once

#include "Shapes/ShapeBarrier.h"

#include "Math/Vector.h"

#include <memory>

struct FTriIndices;

/**
 * A handle to an AGX Dynamics trimesh collision shape.
 *
 * An AGX Dynamics collision trimesh consists of a collection of triangles. Each triangle has three
 * vertices and a single normal. Vertex positions are frequently shared among several triangles so
 * the triangle doesn't store the vertex position but instead an index into a vertex position
 * buffer.
 *
 * VertexPositions: TArray<FVector> - One FVector per point in space where a vertex resides.
 * VertexIndices: TArray<uint32> - Three uint32s per triangle. Used to read from VertexPositions.
 * TriangleNormals: TArray<FVector> - One FVector per triangle.
 *
 * In addition to the collision data a trimesh may contain rendering data. The rendering data
 * contains  its own mesh which may be equivalent to the collision mesh, but may also be different.
 * The render data also store vertex positions, vertex indices, and normals, but also texture
 * coordinates. The normals are not stored per triangle but instead per vertex and should be
 * accessed via the vertex indices just like the positions. The same is true for the texture
 * coordinates.
 */
class AGXUNREALBARRIER_API FTrimeshShapeBarrier : public FShapeBarrier
{
public:
	FTrimeshShapeBarrier();
	FTrimeshShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native);
	FTrimeshShapeBarrier(FTrimeshShapeBarrier&& Other);
	virtual ~FTrimeshShapeBarrier() override;

	int32 GetNumPositions() const;

	int32 GetNumIndices() const;

	int32 GetNumTriangles() const;

	/**
	 * One FVector per vertex location. Vertex positions can be shared between
	 * triangles.
	 */
	TArray<FVector> GetVertexPositions() const;

	/**
	 * Mapping from triangles to vertex positions. Each three consecutive
	 * indices form a triangle between the three pointed-to vertex positions.
	 * Several triangles may reference the same vertex position.
	 */
	TArray<uint32> GetVertexIndices() const;

	/**
	 * Per-triangle normal.
	 */
	TArray<FVector> GetTriangleNormals() const;

	/**
	 * The source name is a user-provided string that is stored with the trimesh.
	 * it can be the name of a file on disk from which the mesh data was read,
	 * or some other form of description. May be the empty string.
	 * @return The source name that has been stored with the trimesh.
	 */
	FString GetSourceName() const;

	FGuid GetMeshDataGuid() const;

	void AllocateNative(
		const TArray<FVector>& Vertices, const TArray<FTriIndices>& TriIndices, bool bClockwise,
		const FString& SourceName);

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
		const FString& SourceName;

		AllocationParameters(const FString& InSourceName)
			: SourceName(InSourceName)
		{
		}
	};

	std::weak_ptr<AllocationParameters> TemporaryAllocationParameters;
};
