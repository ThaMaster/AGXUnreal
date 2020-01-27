#pragma once

#include "Shapes/HeightFieldShapeBarrier.h"

class ALandscape;

namespace AGX_HeightFieldUtilities
{
	AGXUNREAL_API FHeightFieldShapeBarrier CreateHeightField(ALandscape& Landscape);

	AGXUNREAL_API int32 GetLandscapeSideSizeInQuads(ALandscape& Landscape);
}
