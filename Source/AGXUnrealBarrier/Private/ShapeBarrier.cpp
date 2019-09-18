#include "ShapeBarrier.h"

#include "AGXRefs.h"

#include <Misc/AssertionMacros.h>

FShapeBarrier::FShapeBarrier()
	: NativeGeometryRef{new FGeometryRef}
	, NativeShapeRef{new FShapeRef}
{
}

FShapeBarrier::~FShapeBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::uniue_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FShapeRef.
}

bool FShapeBarrier::HasNative() const
{
	return NativeGeometryRef->Native != nullptr && NativeShapeRef->Native != nullptr;
}

void FShapeBarrier::AllocateNative()
{
	check(!HasNative());
	NativeGeometryRef->Native = new agxCollide::Geometry();
	NativeShapeRef->Native = AllocateNativeShape()->Native;
	NativeGeometryRef->Native->add(NativeShapeRef->Native);
}

FGeometryRef* FShapeBarrier::GetNativeGeometry()
{
	return NativeGeometryRef.get();
}

FShapeRef* FShapeBarrier::GetNativeShape()
{
	return NativeShapeRef.get();
}
