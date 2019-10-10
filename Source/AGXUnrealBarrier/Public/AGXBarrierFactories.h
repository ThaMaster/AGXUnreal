#pragma once

#include "RigidBodyBarrier.h"
#include "BoxShapeBarrier.h"
#include "SphereShapeBarrier.h"

#include "AGXRefs.h"

#include <agx/RigidBody.h>

FRigidBodyBarrier CreateRigidBodyBarrier(agx::RigidBody* Body);

FBoxShapeBarrier CreateBoxShapeBarrier(agxCollide::Box* Box);

FSphereShapeBarrier CreateSphereShapeBarrier(agxCollide::Sphere* Sphere);
