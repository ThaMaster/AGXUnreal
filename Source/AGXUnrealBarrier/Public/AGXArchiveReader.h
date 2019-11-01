#pragma once

#include "Containers/UnrealString.h"
#include "Containers/Array.h"

#include <memory>

class FRigidBodyBarrier;
class FBoxShapeBarrier;
class FSphereShapeBarrier;

#define AGX_IMPORT_INSTANTIATOR 1
#define AGX_IMPORT_COLLECTION 2
#define AGX_IMPORT AGX_IMPORT_INSTANTIATOR

#if AGX_IMPORT == AGX_IMPORT_INSTANTIATOR
class FAGXArchiveBody;

class AGXUNREALBARRIER_API FAGXArchiveInstantiator
{
public:
	virtual FAGXArchiveBody* InstantiateBody(const FRigidBodyBarrier& Barrier) = 0;
	virtual ~FAGXArchiveInstantiator();
};


class AGXUNREALBARRIER_API FAGXArchiveBody
{
public:
	virtual void CreateSphere(const FSphereShapeBarrier& Sphere) = 0;
	virtual void CreateBox(const FBoxShapeBarrier& Box) = 0;
	virtual ~FAGXArchiveBody();
};

namespace FAGXArchiveReader
{
	/// \todo Should FAGXArchiveInstantiator and FAGXArchiveBody be moved into
	/// this namespace?

	/// \todo Loop over bodies, for each body call InstantiateBody. Loop over
	/// Geometries/Shapes in body, for each Geometry/Shape, call
	/// Create(Sphere)|(Box).
	AGXUNREALBARRIER_API void Read(const FString& Filename, FAGXArchiveInstantiator& Instantiator);
};
#endif


#if AGX_IMPORT == AGX_IMPORT_COLLECTION

/// \todo These structs doesn't scale to bodies with multiple shapes, which is
/// supported by AGXUnreal. Find a better way.

struct AGXUNREALBARRIER_API FBoxBody
{
	FRigidBodyBarrier* Body;
	FBoxShapeBarrier* Box;
};

struct AGXUNREALBARRIER_API FSphereBody
{
	FRigidBodyBarrier* Body;
	FSphereShapeBarrier* Sphere;
};

class AGXUNREALBARRIER_API FAGXArchiveReader
{
public:
	FAGXArchiveReader(const FString& Filename);
	~FAGXArchiveReader();

	const TArray<FBoxBody>& GetBoxBodies() const;
	const TArray<FSphereBody>& GetSphereBodies() const;
private:
	std::unique_ptr<struct FAGXArchiveContents> Contents;
};

#endif
