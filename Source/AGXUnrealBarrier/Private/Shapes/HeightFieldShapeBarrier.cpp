#include "Shapes/HeightFieldShapeBarrier.h"

#include "AGXRefs.h"
#include "ShapeBarrierImpl.h"
#include "TypeConversions.h"

namespace
{
	agxCollide::HeightField* NativeHeightField(FHeightFieldShapeBarrier* Barrier)
	{
		return Barrier->GetNative()->NativeShape->as<agxCollide::HeightField>();
	}

	const agxCollide::HeightField* NativeHeightField(const FHeightFieldShapeBarrier* Barrier)
	{
		return Barrier->GetNative()->NativeShape->as<agxCollide::HeightField>();
	}
}

FHeightFieldShapeBarrier::FHeightFieldShapeBarrier()
	: FShapeBarrier()
{
}

FHeightFieldShapeBarrier::FHeightFieldShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native)
	: FShapeBarrier(std::move(Native))
{
	check(NativeRef->NativeShape->is<agxCollide::HeightField>());
}

FHeightFieldShapeBarrier::~FHeightFieldShapeBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FHeightFieldShapeRef.
}

void FHeightFieldShapeBarrier::AllocateNative(
	int32 NumVerticesX, int32 NumVerticesY, float SizeX, float SizeY, const TArray<float>& Heights)
{
	FShapeBarrier::AllocateNative([=, &Heights]() {
		this->AllocateNativeHeightField(NumVerticesX, NumVerticesY, SizeX, SizeY, Heights);
	});
}

void FHeightFieldShapeBarrier::AllocateNativeHeightField(
	int32 NumVerticesX, int32 NumVerticesY, float SizeX, float SizeY, const TArray<float>& Heights)
{
	check(!HasNative());
	check(Heights.Num() >= 0);

	agx::Real SizeXAGX = ConvertDistance(SizeX);
	agx::Real SizeYAGX = ConvertDistance(SizeY);

	NativeRef->NativeShape = new agxCollide::HeightField(
		static_cast<size_t>(NumVerticesX), static_cast<size_t>(NumVerticesY), SizeXAGX, SizeYAGX);

	agx::VectorPOD<agx::Real> HeightsAGX;
	HeightsAGX.reserve(static_cast<size_t>(Heights.Num()));

	for (auto& Height : Heights)
	{
		HeightsAGX.push_back(ConvertDistance(Height));
	}

	NativeHeightField(this)->setHeights(HeightsAGX);
}

void FHeightFieldShapeBarrier::AllocateNativeShape()
{
	checkNoEntry();
}

void FHeightFieldShapeBarrier::ReleaseNativeShape()
{
	NativeRef->NativeShape = nullptr;
}
