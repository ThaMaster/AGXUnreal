#include "AGXArchiveReader.h"

#include "RigidBodyBarrier.h"
#include "Shapes/BoxShapeBarrier.h"
#include "Shapes/SphereShapeBarrier.h"
#include "TypeConversions.h"

#include "AGXBarrierFactories.h"

#include "BeginAGXIncludes.h"
#include <agx/RigidBody.h>
#include <agx/Encoding.h>
#include <agxSDK/Simulation.h>
#include <agxCollide/Geometry.h>
#include <agxCollide/Box.h>
#include <agxCollide/Trimesh.h>
#include "EndAGXIncludes.h"


namespace
{
	void InstantiateShapes(const agxCollide::ShapeRefVector& Shapes, FAGXArchiveBody& ArchiveBody)
	{
		for (const agxCollide::ShapeRef& Shape : Shapes)
		{
			switch(Shape->getType())
			{
				case agxCollide::Shape::SPHERE:
				{
					agxCollide::Sphere* Sphere {Shape->as<agxCollide::Sphere>()};
					ArchiveBody.InstantiateSphere(CreateSphereShapeBarrier(Sphere));
					break;
				}
				case agxCollide::Shape::BOX:
				{
					agxCollide::Box* Box {Shape->as<agxCollide::Box>()};
					ArchiveBody.InstantiateBox(CreateBoxShapeBarrier(Box));
					break;
				}
				case agxCollide::Shape::TRIMESH:
				{
					agxCollide::Trimesh* Trimesh {Shape->as<agxCollide::Trimesh>()};
					ArchiveBody.InstantiateTrimesh(CreateTrimeshShapeBarrier(Trimesh));
					break;
				}
				case agxCollide::Shape::GROUP:
				{
					agxCollide::ShapeGroup* Group {Shape->as<agxCollide::ShapeGroup>()};
					InstantiateShapes(Group->getChildren(), ArchiveBody);
					break;
				}
			}
		}
	}
}

void FAGXArchiveReader::Read(const FString& Filename, FAGXArchiveInstantiator& Instantiator)
{
	agxSDK::SimulationRef Simulation {new agxSDK::Simulation()};
	size_t NumRead {Simulation->read(Convert(Filename))};
	if (NumRead == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Could not read .agx filel %s."), *Filename);
		return;
	}

	agx::RigidBodyRefVector& Bodies {Simulation->getRigidBodies()};
	if (Bodies.size() > size_t(std::numeric_limits<int32>::max()))
	{
		UE_LOG(LogTemp, Log, TEXT(".agx file %s contains too many bodies."), *Filename);
		return;
	}

	for (agx::RigidBodyRef& Body : Bodies)
	{
		FRigidBodyBarrier BodyBarrier {CreateRigidBodyBarrier(Body)};
		std::unique_ptr<FAGXArchiveBody> ArchiveBody {Instantiator.InstantiateBody(BodyBarrier)};
		const agxCollide::GeometryRefVector& Geometries {Body->getGeometries()};
		for (const agxCollide::GeometryRef& Geometry : Geometries)
		{
			::InstantiateShapes(Geometry->getShapes(), *ArchiveBody);
		}
	}
}
