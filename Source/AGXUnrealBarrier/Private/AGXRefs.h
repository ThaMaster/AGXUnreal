#pragma once

#include "BeginAGXIncludes.h"
#include <agx/Constraint.h>
#include <agx/Material.h>
#include <agx/RigidBody.h>
#include <agxSDK/Simulation.h>
#include <agxCollide/Geometry.h>
#include <agxCollide/Shape.h>
#include "EndAGXIncludes.h"

struct FSimulationRef
{
	agxSDK::SimulationRef Native;

	FSimulationRef() = default;
	FSimulationRef(agxSDK::Simulation* InNative)
		: Native(InNative)
	{
	}
};

struct FRigidBodyRef
{
	agx::RigidBodyRef Native;

	FRigidBodyRef() = default;
	FRigidBodyRef(agx::RigidBody* InNative)
		: Native(InNative)
	{
	}
};

struct FGeometryRef
{
	agxCollide::GeometryRef Native;

	FGeometryRef() = default;
	FGeometryRef(agxCollide::Geometry* InNative)
		: Native(InNative)
	{
	}
};

struct FShapeRef
{
	agxCollide::ShapeRef Native;

	FShapeRef() = default;
	FShapeRef(agxCollide::Shape* InNative)
		: Native(InNative)
	{
	}
};

struct FGeometryAndShapeRef
{
	agxCollide::GeometryRef NativeGeometry;
	agxCollide::ShapeRef NativeShape;

	FGeometryAndShapeRef() = default;
	FGeometryAndShapeRef(agxCollide::Geometry* InNativeGeometry, agxCollide::Shape* InNativeShape)
		: NativeGeometry(InNativeGeometry)
		, NativeShape(InNativeShape)
	{
	}
};

struct FConstraintRef
{
	agx::ConstraintRef Native;

	FConstraintRef() = default;
	FConstraintRef(agx::Constraint* InNative)
		: Native(InNative)
	{
	}
};

struct FMaterialRef
{
	agx::MaterialRef Native;

	FMaterialRef() = default;
	FMaterialRef(agx::Material* InNative)
		: Native(InNative)
	{
	}
};
