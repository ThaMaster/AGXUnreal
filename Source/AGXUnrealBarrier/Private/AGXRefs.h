#pragma once

#include "BeginAGXIncludes.h"
#include <agx/Constraint.h>
#include <agx/LockJoint.h>
#include <agx/Hinge.h>
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

struct FBoxShapeRef
{
	agxCollide::BoxRef Native;
};

struct FSphereShapeRef
{
	agxCollide::SphereRef Native;
};

struct FConstraintRef
{
	agx::ConstraintRef Native;
};
