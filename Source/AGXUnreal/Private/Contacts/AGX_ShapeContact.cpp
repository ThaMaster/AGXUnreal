#include "Contacts/AGX_ShapeContact.h"

// AGX Dynamics for Unreal includes.
#include "Shapes/AGX_ShapeComponent.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_LogCategory.h"

FAGX_ShapeContact::FAGX_ShapeContact(FShapeContactBarrier&& InBarrier)
	: Barrier(std::move(InBarrier))
{
}

bool FAGX_ShapeContact::HasNative() const
{
	return Barrier.HasNative();
}

bool FAGX_ShapeContact::IsEnabled() const
{
	check(HasNative());
	return Barrier.IsEnabled();
}

FRigidBodyBarrier FAGX_ShapeContact::GetBody1() const
{
	check(HasNative());
	return Barrier.GetBody1();
}

FRigidBodyBarrier FAGX_ShapeContact::GetBody2() const
{
	check(HasNative());
	return Barrier.GetBody2();
}

FEmptyShapeBarrier FAGX_ShapeContact::GetShape1() const
{
	check(HasNative());
	return Barrier.GetShape1();
}

FEmptyShapeBarrier FAGX_ShapeContact::GetShape2() const
{
	check(HasNative());
	return Barrier.GetShape2();
}


int32 FAGX_ShapeContact::GetNumContacts() const
{
	check(HasNative());
	return Barrier.GetNumContacts();
}

TArray<FAGX_ContactPoint> FAGX_ShapeContact::GetContactPoints() const
{
	check(HasNative());
	TArray<FContactPointBarrier> ContactPointBarriers = Barrier.GetContactPoints();
	TArray<FAGX_ContactPoint> ContactPoints;
	ContactPoints.Reserve(ContactPointBarriers.Num());
	for (FContactPointBarrier& ContactPointBarrier : ContactPointBarriers)
	{
		ContactPoints.Emplace(std::move(ContactPointBarrier));
	}
	return ContactPoints;
}

int UAGX_ShapeContact_FL::GetNumPoints(UPARAM(ref) FAGX_ShapeContact& ShapeContactRef)
{
	// UFUNCTIONS does not support size_t or uint32, so the best we can do is to give a warning
	// if the numbe of points are above the maximum value for an int.

	return ShapeContactRef.GetNumContacts();
}

namespace
{
	bool IsMatch(UAGX_ShapeComponent* Shape, const FGuid& Guid)
	{
		if (!Shape || !Shape->HasNative() || !Guid.IsValid())
		{
			return false;
		}

		return Shape->GetNative()->GetGeometryGuid() == Guid;
	}

	bool IsMatch(UAGX_RigidBodyComponent* Body, const FGuid& Guid)
	{
		if (!Body || !Body->HasNative() || !Guid.IsValid())
		{
			return false;
		}

		return Body->GetNative()->GetGuid() == Guid;
	}

	template <typename T>
	T* GetFromGuid(const FGuid& Guid)
	{
		for (TObjectIterator<T> ObjectIt; ObjectIt; ++ObjectIt)
		{
			T* Obj = *ObjectIt;
			if (IsMatch(Obj, Guid))
			{
				return Obj;
			}
		}

		return nullptr;
	}
}

UAGX_ShapeComponent* UAGX_ShapeContact_FL::GetFirstShape(UPARAM(ref)
															 FAGX_ShapeContact& ShapeContactRef)
{
	FEmptyShapeBarrier Shape = ShapeContactRef.GetShape1();
	FGuid Guid = Shape.GetGuid();
	return GetFromGuid<UAGX_ShapeComponent>(Guid);
}

#if 0
UAGX_ShapeComponent* UAGX_ShapeContact_FL::GetFirstShape(UPARAM(ref)
															 FAGX_ShapeContact& ShapeContactRef)
{
	return GetFromGuid<UAGX_ShapeComponent>(ShapeContactRef.Data.FirstShapeGuid);
}

UAGX_ShapeComponent* UAGX_ShapeContact_FL::GetSecondShape(UPARAM(ref)
															  FAGX_ShapeContact& ShapeContactRef)
{
	return GetFromGuid<UAGX_ShapeComponent>(ShapeContactRef.Data.SecondShapeGuid);
}

UAGX_RigidBodyComponent* UAGX_ShapeContact_FL::GetFirstBody(UPARAM(ref)
																FAGX_ShapeContact& ShapeContactRef)
{
	return GetFromGuid<UAGX_RigidBodyComponent>(ShapeContactRef.Data.FirstBodyGuid);
}

UAGX_RigidBodyComponent* UAGX_ShapeContact_FL::GetSecondBody(UPARAM(ref)
																 FAGX_ShapeContact& ShapeContactRef)
{
	return GetFromGuid<UAGX_RigidBodyComponent>(ShapeContactRef.Data.SecondBodyGuid);
}

int UAGX_ShapeContact_FL::GetNumPoints(UPARAM(ref) FAGX_ShapeContact& ShapeContactRef)
{
	// UFUNCTIONS does not support size_t or uint32, so the best we can do is to give a warning
	// if the numbe of points are above the maximum value for an int.
	size_t NumPoints = ShapeContactRef.Data.Points.Num();
	if (NumPoints > std::numeric_limits<int>::max())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("GetNumPoints: The number of contact points is too large for int. Returning "
				 "std::numeric_limits<int>::max()."));
		return std::numeric_limits<int>::max();
	}

	return static_cast<int>(NumPoints);
}

FVector UAGX_ShapeContact_FL::GetPointPosition(
	UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int PointIndex)
{
	if (PointIndex >= ShapeContactRef.Data.Points.Num())
	{
		UE_LOG(LogAGX, Error, TEXT("GetPointPosition: tried to access point index out of bounds."));
		return FVector::ZeroVector;
	}

	return ShapeContactRef.Data.Points[PointIndex].Position;
}

FVector UAGX_ShapeContact_FL::GetPointForce(
	UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int PointIndex)
{
	if (PointIndex >= ShapeContactRef.Data.Points.Num())
	{
		UE_LOG(LogAGX, Error, TEXT("GetPointForce: tried to access point index out of bounds."));
		return FVector::ZeroVector;
	}

	return ShapeContactRef.Data.Points[PointIndex].Force;
}

FVector UAGX_ShapeContact_FL::GetPointNormalForce(
	UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int PointIndex)
{
	if (PointIndex >= ShapeContactRef.Data.Points.Num())
	{
		UE_LOG(
			LogAGX, Error, TEXT("GetPointNormalForce: tried to access point index out of bounds."));
		return FVector::ZeroVector;
	}

	return ShapeContactRef.Data.Points[PointIndex].NomalForce;
}

FVector UAGX_ShapeContact_FL::GetPointNormal(
	UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int PointIndex)
{
	if (PointIndex >= ShapeContactRef.Data.Points.Num())
	{
		UE_LOG(LogAGX, Error, TEXT("GetPointNormal: tried to access point index out of bounds."));
		return FVector::ZeroVector;
	}

	return ShapeContactRef.Data.Points[PointIndex].Normal;
}

float UAGX_ShapeContact_FL::GetPointDepth(
	UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int PointIndex)
{
	if (PointIndex >= ShapeContactRef.Data.Points.Num())
	{
		UE_LOG(LogAGX, Error, TEXT("GetPointDepth: tried to access point index out of bounds."));
		return 0.f;
	}

	return ShapeContactRef.Data.Points[PointIndex].Depth;
}

float UAGX_ShapeContact_FL::GetPointArea(
	UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int PointIndex)
{
	if (PointIndex >= ShapeContactRef.Data.Points.Num())
	{
		UE_LOG(LogAGX, Error, TEXT("GetPointArea: tried to access point index out of bounds."));
		return 0.f;
	}

	return ShapeContactRef.Data.Points[PointIndex].Area;
}
#endif
