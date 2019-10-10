#include "AGXBarrierFactories.h"

FRigidBodyBarrier CreateRigidBodyBarrier(agx::RigidBody* Body)
{
	FRigidBodyBarrier Barrier(std::unique_ptr<FRigidBodyRef>{new FRigidBodyRef{Body}});
	return Barrier;
}

FBoxShapeBarrier CreateBoxShapeBarrier(agxCollide::Box* Box)
{
	FBoxShapeBarrier Barrier(std::unique_ptr<FGeometryAndShapeRef>{new FGeometryAndShapeRef{Box->getGeometry(), Box}});
	return Barrier;
}

FSphereShapeBarrier CreateSphereShapeBarrier(agxCollide::Sphere* Sphere)
{
	FSphereShapeBarrier Barrier(
		std::unique_ptr<FGeometryAndShapeRef>{new FGeometryAndShapeRef{Sphere->getGeometry(), Sphere}});
	return Barrier;
}
