
#pragma push_macro("PI")
#pragma push_macro("verify")

#undef PI
#undef verify

#include <agx/RigidBody.h>

#pragma pop_macro("PI")
#pragma pop_macro("verify")

struct FRigidBodyRef
{
	agx::RigidBodyRef Native;
};
