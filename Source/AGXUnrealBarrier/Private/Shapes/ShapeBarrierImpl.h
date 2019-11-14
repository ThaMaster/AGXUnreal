
#include "ShapeBarrier.h"

#include "AGXRefs.h"

#include <utility>

template<typename TFunc, typename... TPack>
void FShapeBarrier::AllocateNative(TFunc Factory, TPack... Params)
{
	/// \todo This almost copy/paste from the non-templated version. Find a way to
	/// call one from the other, or some other way to share implementation.
	check(!HasNative());
	NativeRef->NativeGeometry = new agxCollide::Geometry();
	Factory(std::forward<TPack>(Params)...);
	NativeRef->NativeGeometry->add(NativeRef->NativeShape);
}
