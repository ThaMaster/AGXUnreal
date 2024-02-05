// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Misc/AssertionMacros.h"

#if defined(AGXUNREAL_CHECK) && AGXUNREAL_CHECK == 1
#define AGX_CHECK(x) check(x)
#else
#define AGX_CHECK(x)
#endif
