#include "Utilities/AGX_ShapeUtilities.h"

// AGX Dynamics for Unreal includes.
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxUtil/TrimeshHelperFunctions.h>
#include "EndAGXIncludes.h"


bool FAGX_ShapeUtilities::ComputeOrientedBox(
	const TArray<FVector>& Vertices, FVector& OutHalfExtents, FTransform& OutTransform)
{
	agx::Vec3Vector VerticesAGX;
	VerticesAGX.reserve(Vertices.Num());

	for (const FVector& Vertex : Vertices)
	{
		VerticesAGX.push_back(ConvertDisplacement(Vertex));
	}

	agx::Vec3 halfExtentsAGX;
	agx::AffineMatrix4x4 transformAGX;

	if(!agxUtil::computeOrientedBox(VerticesAGX, halfExtentsAGX, transformAGX))
	{
		return false;
	}

	OutHalfExtents = ConvertDisplacement(halfExtentsAGX);
	OutTransform = Convert(transformAGX);
	return true;
}
