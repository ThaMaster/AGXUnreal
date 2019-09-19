#pragma once

#include <Math/Vector.h>

#include <agx/Vec3.h>

inline float Convert(agx::Real V)
{
	return static_cast<float>(V);
}

inline agx::Real Convert(float V)
{
	return static_cast<agx::Real>(V);
}

inline FVector Convert(agx::Vec3 V)
{
	return FVector(Convert(V.x()), Convert(V.y()), Convert(V.z()));
}

inline agx::Vec3 Convert(FVector V)
{
	return agx::Vec3(Convert(V.X), Convert(V.Y), Convert(V.Z));
}
