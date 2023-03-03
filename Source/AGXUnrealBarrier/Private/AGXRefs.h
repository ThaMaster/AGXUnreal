// Copyright 2023, Algoryx Simulation AB.

#pragma once

#include "BeginAGXIncludes.h"
#include <agx/Constraint.h>
#include <agx/ElementaryConstraint.h>
#include <agx/FrictionModel.h>
#include <agx/Material.h>
#include <agx/MassProperties.h>
#include <agx/ref_ptr.h>
#include <agx/RigidBody.h>
#include <agxSDK/MergeSplitHandler.h>
#include <agxSDK/MergeSplitThresholds.h>
#include <agxSDK/Simulation.h>
#include <agxCollide/Geometry.h>
#include <agxCollide/Shape.h>
#include <agxTerrain/Terrain.h>
#include <agxTerrain/TerrainMaterial.h>
#include <agxTerrain/TerrainPager.h>
#include <agxModel/Tire.h>
#include "EndAGXIncludes.h"

#include "AGXNotify.h"

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

struct FMassPropertiesPtr
{
	agx::MassProperties* Native = nullptr;

	FMassPropertiesPtr() = default;
	FMassPropertiesPtr(agx::MassProperties* InNative)
		: Native(InNative)
	{
	}
};

struct FMergeSplitPropertiesPtr
{
	agxSDK::MergeSplitProperties* Native = nullptr;

	FMergeSplitPropertiesPtr() = default;
	FMergeSplitPropertiesPtr(agxSDK::MergeSplitProperties* InNative)
		: Native(InNative)
	{
	}
};

struct FMergeSplitThresholdsRef
{
	agxSDK::MergeSplitThresholdsRef Native;

	FMergeSplitThresholdsRef() = default;
	FMergeSplitThresholdsRef(agxSDK::MergeSplitThresholds* InNative)
		: Native(InNative)
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

struct FConstraintControllerRef
{
	agx::ref_ptr<agx::BasicControllerConstraint> Native;
	FConstraintControllerRef() = default;
	FConstraintControllerRef(agx::BasicControllerConstraint* InNative)
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

struct FContactMaterialRef
{
	agx::ContactMaterialRef Native;

	FContactMaterialRef() = default;
	FContactMaterialRef(agx::ContactMaterial* InNative)
		: Native(InNative)
	{
	}
};

struct FTerrainMaterialRef
{
	agxTerrain::TerrainMaterialRef Native;

	FTerrainMaterialRef() = default;
	FTerrainMaterialRef(agxTerrain::TerrainMaterial* InNative)
		: Native(InNative)
	{
	}
};

struct FTerrainRef
{
	agxTerrain::TerrainRef Native;

	FTerrainRef() = default;
	FTerrainRef(agxTerrain::Terrain* InNative)
		: Native(InNative)
	{
	}
};

struct FTerrainPagerRef
{
	// @todo: once AGX Dynamics exposes a TerrainPagerRef, use that.
	agx::ref_ptr<agxTerrain::TerrainPager> Native;

	FTerrainPagerRef() = default;
	FTerrainPagerRef(agxTerrain::TerrainPager* InNative)
		: Native(InNative)
	{
	}
};

struct FShovelRef
{
	agxTerrain::ShovelRef Native;

	FShovelRef() = default;
	FShovelRef(agxTerrain::Shovel* InNative)
		: Native(InNative)
	{
	}
};

struct FNotifyRef
{
	agx::ref_ptr<FAGXNotify> Native;

	FNotifyRef() = default;
	FNotifyRef(FAGXNotify* InNative)
		: Native(InNative)
	{
	}
};

struct FTireRef
{
	agxModel::TireRef Native;

	FTireRef() = default;
	FTireRef(agxModel::Tire* InNative)
		: Native(InNative)
	{
	}
};
