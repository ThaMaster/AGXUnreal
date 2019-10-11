#pragma once

#include "Containers/UnrealString.h"
#include "Containers/Array.h"

#include <memory>

class FRigidBodyBarrier;
class FBoxShapeBarrier;
class FSphereShapeBarrier;

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
