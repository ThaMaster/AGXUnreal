#include "BeginAGXIncludes.h"
#include <agx/RigidBody.h>
#include <agxCollide/Geometry.h>
#include <agxCollide/Shape.h>
#include <agxCollide/Box.h>
#include "EndAGXIncludes.h"

struct FRigidBodyRef
{
	agx::RigidBodyRef Native;
};

struct FGeometryRef
{
	agxCollide::GeometryRef Native;
};

struct FShapeRef
{
	agxCollide::ShapeRef Native;
};

struct FBoxShapeRef
{
	agxCollide::BoxRef Native;
};
