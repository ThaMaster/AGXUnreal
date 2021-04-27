#if 0
#include "BeginAGXIncludes.h"
#include <agxWire/RenderIterator.h>
#include "EndAGXIncludes.h"

struct FWireRenderIterator
{
	agxWire::RenderIterator Native;

	FWireRenderIterator() = default;

	FWireRenderIterator(const agxWire::RenderIterator& InNative)
		: Native(InNative)
	{
	}
};
#endif
