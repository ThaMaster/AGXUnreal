#include "AGXBarrierFactories.h"

#include <agx/RigidBody.h>

#include "BeginAGXIncludes.h"
#include <agxCollide/Sphere.h>
#include <agxCollide/Box.h>
#include <agxCollide/Trimesh.h>
#include <EndAGXIncludes.h>

#include <memory>

FRigidBodyBarrier CreateRigidBodyBarrier(agx::RigidBody* Body)
{
	return {std::make_unique<FRigidBodyRef>(Body)};
}

FSphereShapeBarrier CreateSphereShapeBarrier(agxCollide::Sphere* Sphere)
{
	return {std::make_unique<FGeometryAndShapeRef>(Sphere->getGeometry(), Sphere)};
}

FBoxShapeBarrier CreateBoxShapeBarrier(agxCollide::Box* Box)
{
	return {std::make_unique<FGeometryAndShapeRef>(Box->getGeometry(), Box)};
}

FTrimeshShapeBarrier CreateTrimeshShapeBarrier(agxCollide::Trimesh* Trimesh)
{
	return {std::make_unique<FGeometryAndShapeRef>(Trimesh->getGeometry(), Trimesh)};
}
