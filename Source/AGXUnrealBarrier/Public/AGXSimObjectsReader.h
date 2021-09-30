#pragma once

// AGX Dynamics for Unreal includes.
#include "Utilities/SuccessOrError.h"

// Unreal Engine includes.
#include "Containers/UnrealString.h"
#include "Containers/Array.h"

// System includes.
#include <memory>

class FRigidBodyBarrier;
class FBoxShapeBarrier;
class FSphereShapeBarrier;
class FCylinderShapeBarrier;
class FCapsuleShapeBarrier;
class FTrimeshShapeBarrier;
class FHingeBarrier;
class FPrismaticBarrier;
class FBallJointBarrier;
class FCylindricalJointBarrier;
class FDistanceJointBarrier;
class FLockJointBarrier;
class FShapeMaterialBarrier;
class FContactMaterialBarrier;
class FTwoBodyTireBarrier;
class FWireBarrier;

/*
The separation between the Unreal part and the AGX Dynamics part of the plugin
makes reading .agx archives a bit complicated. The reader consists of two
parts, one that reads and traverses the AGX Dynamics objects and one that
creates AGXUnreal objects corresponding to the AGX Dynamics objects. An object
oriented approach is used where a set of abstract classes and pure virtual
member functions define the interface that the AGX Dynamics side of the reader
uses, while the AGXUnreal side inherits from the abstract classes and in the
implementation of the member functions creates the corresponding AGXUnreal
objects.

The main abstract base class is FAGXSimObjectsInstantiator. An instance of this
class must be passed when reading an AGX Dynamics archive. The Instantiator is
notified of top level objects, such as RigidBodies, found in the AGX Dynamics
archive and the Instantiator is expected to create a persistent representation
of that top level object. A handle, FAGXSimObjectBody for a RigidBody, to the
created object is returned to the archive reader. The object handle is notified
of sub-objects, such as shapes, that are found in the top level object. Barrier
objects are used to pass the AGX Dynamics state from the reader part to the
AGXUnreal part. The Barrier objects are short-lived and should only be used for
the duration of the notification call.
*/

class FAGXSimObjectBody;

/**
 * Struct for holding Rigid Body Archives of TwoBodyTire.
 */
struct FTwoBodyTireArchiveBodies
{
	FTwoBodyTireArchiveBodies() = default;
	FTwoBodyTireArchiveBodies(FAGXSimObjectBody* InTire, FAGXSimObjectBody* InHub)
		: TireBodyArchive {InTire}
		, HubBodyArchive {InHub}
	{
	}
	std::unique_ptr<FAGXSimObjectBody> TireBodyArchive;
	std::unique_ptr<FAGXSimObjectBody> HubBodyArchive;
};

/**
 * Instantiator for sub-objects of RigidBody.
 */
class AGXUNREALBARRIER_API FAGXSimObjectBody
{
public:
	/**
	 * Create a new Sphere corresponding to the given FSphereShapeBarrier.
	 * Does not return anything because a Sphere cannot have subobjects.
	 * @param Sphere The sphere for which a persistent representation should be created.
	 */
	virtual void InstantiateSphere(const FSphereShapeBarrier& Sphere) = 0;

	/**
	 * Create a new Box corresponding to the given FBoxShapeBarrier.
	 * Does not return anything because a Box cannot have subobjects.
	 * @param Box The box for which a persistent representation should be created.
	 */
	virtual void InstantiateBox(const FBoxShapeBarrier& Box) = 0;

	/**
	 * Create a new Cylinder corresponding to the given FCylinderShapeBarrier.
	 * Does not return anything because a Cylinder cannot have subobjects.
	 * @param Cylinder The cylinder for which a persistent representation should be created.
	 */
	virtual void InstantiateCylinder(const FCylinderShapeBarrier& Cylinder) = 0;

	/**
	 * Create a new Capsule corresponding to the given FCapsuleShapeBarrier.
	 * Does not return anything because a Capsule cannot have subobjects.
	 * @param Capsule The Capsule for which a persistent representation should be created.
	 */
	virtual void InstantiateCapsule(const FCapsuleShapeBarrier& Capsule) = 0;

	/**
	 * Create a new Trimesh corresponding to the given FTrimeshShapeBarrier.
	 * Does not return anything because a Trimesh cannot have subobjects.
	 * @param Trimesh The trimesh for which a persistent representation should be created.
	 */
	virtual void InstantiateTrimesh(const FTrimeshShapeBarrier& Trimesh) = 0;

	virtual ~FAGXSimObjectBody() = default;
};

/**
 * Instantiator base class for top-level objects.
 */
class AGXUNREALBARRIER_API FAGXSimObjectsInstantiator
{
public:
	/**
	 * Create a new RigidBody corresponding to the given FRigidBodyBarrier.
	 * @param RigidBody The body for which a persistent representation should be made.
	 * @return A handle to the persistent object through which shapes can be added.
	 */
	virtual FAGXSimObjectBody* InstantiateBody(const FRigidBodyBarrier& RigidBody) = 0;

	virtual void InstantiateHinge(const FHingeBarrier& Hinge) = 0;

	virtual void InstantiatePrismatic(const FPrismaticBarrier& Prismatic) = 0;

	virtual void InstantiateBallJoint(const FBallJointBarrier& BallJoint) = 0;

	virtual void InstantiateCylindricalJoint(const FCylindricalJointBarrier& CylindricalJoint) = 0;

	virtual void InstantiateDistanceJoint(const FDistanceJointBarrier& DistanceJoint) = 0;

	virtual void InstantiateLockJoint(const FLockJointBarrier& LockJoint) = 0;

	virtual void InstantiateSphere(
		const FSphereShapeBarrier& Sphere, FAGXSimObjectBody* Body = nullptr) = 0;

	virtual void InstantiateBox(
		const FBoxShapeBarrier& Box, FAGXSimObjectBody* Body = nullptr) = 0;

	virtual void InstantiateCylinder(
		const FCylinderShapeBarrier& Cylinder, FAGXSimObjectBody* Body = nullptr) = 0;

	virtual void InstantiateCapsule(
		const FCapsuleShapeBarrier& Capsule, FAGXSimObjectBody* Body = nullptr) = 0;

	virtual void InstantiateTrimesh(
		const FTrimeshShapeBarrier& Trimesh, FAGXSimObjectBody* Body = nullptr) = 0;

	virtual void InstantiateShapeMaterial(const FShapeMaterialBarrier& ShapeMaterial) = 0;

	virtual void InstantiateContactMaterial(const FContactMaterialBarrier& ContactMaterial) = 0;

	virtual void DisabledCollisionGroups(
		const TArray<std::pair<FString, FString>>& DisabledGroups) = 0;

	virtual FTwoBodyTireArchiveBodies InstantiateTwoBodyTire(const FTwoBodyTireBarrier& Tire) = 0;

	virtual void InstantiateWire(const FWireBarrier& Wire) = 0;


	virtual ~FAGXSimObjectsInstantiator() = default;
};

namespace FAGXSimObjectsReader
{
	/**
	 * Read the AGX Dynamics archive pointed to by 'Filename' and for each
	 * supported object found call the corresponding Instantiate member function
	 * on the given 'Instantiator' or a handle returned from the 'Instantiator'.
	 * @param Filename Path to the AGX Dynamics archive to read.
	 * @param Instantiator Set of callback functions to call for each object read.
	 * @return True if the archive was read successfully.
	 */
	AGXUNREALBARRIER_API FSuccessOrError
	Read(const FString& Filename, FAGXSimObjectsInstantiator& Instantiator);

	AGXUNREALBARRIER_API FSuccessOrError ReadUrdf(
		const FString& UrdfFilePath, const FString& UrdfPackagePath,
		FAGXSimObjectsInstantiator& Instantiator);
};
