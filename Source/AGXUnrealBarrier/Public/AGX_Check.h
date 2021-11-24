#pragma once

#if defined(AGXUNREAL_CHECK) && AGXUNREAL_CHECK == 1
#	pragma message("AGX_CHECK enabled.")
#	define AGX_CHECK(x) check(x)
#else
#	pragma message("AGX_CHECK disabled.")
#	define AGX_CHECK(x)
#endif
