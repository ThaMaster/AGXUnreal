#pragma once

#include "BeginAGXIncludes.h"
#include <agx/Constraint.h>
#include <agx/RigidBody.h>
#include <agxSDK/Simulation.h>
#include <agxCollide/Geometry.h>
#include <agxCollide/Shape.h>
#include <agxCollide/Box.h>
#include "EndAGXIncludes.h"

struct FSimulationRef
{
	agxSDK::SimulationRef Native;
};

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

struct FGeometryAndShapeRef
{
	agxCollide::GeometryRef NativeGeometry;
	agxCollide::ShapeRef NativeShape;
};

/// \todo These may not be needed, if we use the FGeometryAndShapeRef approach
/// and down-casts everywhere.
#if 0
struct FBoxShapeRef
{
	agxCollide::BoxRef Native;
};

struct FSphereShapeRef
{
	agxCollide::SphereRef Native;
};
#endif

struct FConstraintRef
{
	agx::ConstraintRef Native;
};
