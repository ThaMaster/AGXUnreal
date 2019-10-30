#pragma once

#include "RigidBodyBarrier.h"
#include "Shapes/BoxShapeBarrier.h"
#include "Shapes/SphereShapeBarrier.h"

#include "AGXRefs.h"

#include <agx/RigidBody.h>

FRigidBodyBarrier CreateRigidBodyBarrier(agx::RigidBody* Body);

FBoxShapeBarrier CreateBoxShapeBarrier(agxCollide::Box* Box);

FSphereShapeBarrier CreateSphereShapeBarrier(agxCollide::Sphere* Sphere);
