// Copyright 2021, Algoryx Simulation AB.


#pragma once

#if defined(AGXUNREAL_CHECK) && AGXUNREAL_CHECK == 1
#	define AGX_CHECK(x) check(x)
#else
#	define AGX_CHECK(x)
#endif
