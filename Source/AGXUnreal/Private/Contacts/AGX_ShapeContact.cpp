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

namespace
{
	bool TestHasNative(const FAGX_ShapeContact& ShapeContact, const TCHAR* AttributeName)
	{
		if (ShapeContact.HasNative())
		{
			return true;
		}
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot get %s from a ShapeContact that doesn't have a native AGX Dynamics "
				 "representation"),
			AttributeName);
		return false;
	}
}

bool FAGX_ShapeContact::IsEnabled() const
{
	if (!TestHasNative(*this, TEXT("Enabled")))
	{
		return false;
	}
	return Barrier.IsEnabled();
}

FRigidBodyBarrier FAGX_ShapeContact::GetBody1() const
{
	if (!TestHasNative(*this, TEXT("First Body")))
	{
		return FRigidBodyBarrier();
	}
	return Barrier.GetBody1();
}

FRigidBodyBarrier FAGX_ShapeContact::GetBody2() const
{
	if (!TestHasNative(*this, TEXT("Second Body")))
	{
		return FRigidBodyBarrier();
	}
	return Barrier.GetBody2();
}

FEmptyShapeBarrier FAGX_ShapeContact::GetShape1() const
{
	if (!TestHasNative(*this, TEXT("First Shape")))
	{
		return FEmptyShapeBarrier();
	}
	return Barrier.GetShape1();
}

FEmptyShapeBarrier FAGX_ShapeContact::GetShape2() const
{
	if (!TestHasNative(*this, TEXT("Second Shape")))
	{
		return FEmptyShapeBarrier();
	}
	return Barrier.GetShape2();
}

FVector FAGX_ShapeContact::CalculateRelativeVelocity(int32 PointIndex) const
{
	if (!TestHasNative(*this, TEXT("RelativeVelocity")))
	{
		return FVector::ZeroVector;
	}
	return Barrier.CalculateRelativeVelocity(PointIndex);
}

int32 FAGX_ShapeContact::GetNumContactPoints() const
{
	if (!TestHasNative(*this, TEXT("Num Contact Points")))
	{
		return 0;
	}
	return Barrier.GetNumContactPoints();
}

TArray<FAGX_ContactPoint> FAGX_ShapeContact::GetContactPoints() const
{
	if (!TestHasNative(*this, TEXT("Contact Points")))
	{
		return TArray<FAGX_ContactPoint>();
	}
	TArray<FContactPointBarrier> ContactPointBarriers = Barrier.GetContactPoints();
	TArray<FAGX_ContactPoint> ContactPoints;
	ContactPoints.Reserve(ContactPointBarriers.Num());
	for (FContactPointBarrier& ContactPointBarrier : ContactPointBarriers)
	{
		ContactPoints.Emplace(std::move(ContactPointBarrier));
	}
	return ContactPoints;
}

FAGX_ContactPoint FAGX_ShapeContact::GetContactPoint(int32 Index) const
{
	if (!TestHasNative(*this, TEXT("Contact Point")))
	{
		return FAGX_ContactPoint();
	}
	return Barrier.GetContactPoint(Index);
}

/*
 * Function Library implementation starts here.
 */

namespace
{
	bool IsValidPointIndex(
		const FAGX_ShapeContact& ShapeContact, int32 PointIndex, const TCHAR* AttributeName)
	{
		const int32 NumContactPoints = ShapeContact.GetNumContactPoints();
		if (PointIndex >= 0 && PointIndex < NumContactPoints)
		{
			return true;
		}
		UE_LOG(
			LogAGX, Error,
			TEXT("Trying to get %s from contact point at index %d in a ShapeContact that only has "
				 "%d contact points."),
			AttributeName, PointIndex, NumContactPoints);
		return false;
	}

	bool CheckHasNativeAndValidPointIndex(
		const FAGX_ShapeContact& ShapeContact, int32 PointIndex, const TCHAR* AttributeName)
	{
		return TestHasNative(ShapeContact, AttributeName) &&
			   IsValidPointIndex(ShapeContact, PointIndex, AttributeName);
	}

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
	if (!TestHasNative(ShapeContactRef, TEXT("First Shape")))
	{
		return nullptr;
	}
	return GetFromGuid<UAGX_ShapeComponent>(ShapeContactRef.GetShape1().GetGeometryGuid());
}

UAGX_ShapeComponent* UAGX_ShapeContact_FL::GetSecondShape(UPARAM(ref)
															  FAGX_ShapeContact& ShapeContactRef)
{
	if (!TestHasNative(ShapeContactRef, TEXT("Second Shape")))
	{
		return nullptr;
	}
	return GetFromGuid<UAGX_ShapeComponent>(ShapeContactRef.GetShape2().GetGeometryGuid());
}

UAGX_RigidBodyComponent* UAGX_ShapeContact_FL::GetFirstBody(UPARAM(ref)
																FAGX_ShapeContact& ShapeContactRef)
{
	if (!TestHasNative(ShapeContactRef, TEXT("First Body")))
	{
		return nullptr;
	}
	return GetFromGuid<UAGX_RigidBodyComponent>(ShapeContactRef.GetBody1().GetGuid());
}

UAGX_RigidBodyComponent* UAGX_ShapeContact_FL::GetSecondBody(UPARAM(ref)
																 FAGX_ShapeContact& ShapeContactRef)
{
	if (!TestHasNative(ShapeContactRef, TEXT("Second Body")))
	{
		return nullptr;
	}
	return GetFromGuid<UAGX_RigidBodyComponent>(ShapeContactRef.GetBody2().GetGuid());
}

FVector UAGX_ShapeContact_FL::CalculateRelativeVelocity(
	UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int32 PointIndex)
{
	if (!TestHasNative(ShapeContactRef, TEXT("RelativeVelocity")))
	{
		return FVector::ZeroVector;
	}
	return ShapeContactRef.CalculateRelativeVelocity(PointIndex);
}

int32 UAGX_ShapeContact_FL::GetNumContactPoints(UPARAM(ref) FAGX_ShapeContact& ShapeContactRef)
{
	if (!TestHasNative(ShapeContactRef, TEXT("Num Contact Points")))
	{
		return 0;
	}
	return ShapeContactRef.GetNumContactPoints();
}

float UAGX_ShapeContact_FL::GetPointDepth(
	UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int32 PointIndex)
{
	if (!CheckHasNativeAndValidPointIndex(ShapeContactRef, PointIndex, TEXT("Depth")))
	{
		return 0.0f;
	}
	return ShapeContactRef.GetContactPoint(PointIndex).GetDepth();
}

FVector UAGX_ShapeContact_FL::GetPointLocation(
	UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int32 PointIndex)
{
	if (!CheckHasNativeAndValidPointIndex(ShapeContactRef, PointIndex, TEXT("Location")))
	{
		return FVector::ZeroVector;
	}
	return ShapeContactRef.GetContactPoint(PointIndex).GetLocation();
}

FVector UAGX_ShapeContact_FL::GetPointForce(
	UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int32 PointIndex)
{
	if (!CheckHasNativeAndValidPointIndex(ShapeContactRef, PointIndex, TEXT("Force")))
	{
		return FVector::ZeroVector;
	}
	return ShapeContactRef.GetContactPoint(PointIndex).GetForce();
}

FVector UAGX_ShapeContact_FL::GetPointNormalForce(
	UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int32 PointIndex)
{
	if (!CheckHasNativeAndValidPointIndex(ShapeContactRef, PointIndex, TEXT("Normal Force")))
	{
		return FVector::ZeroVector;
	}
	return ShapeContactRef.GetContactPoint(PointIndex).GetNormalForce();
}

FVector UAGX_ShapeContact_FL::GetPointNormal(
	UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int32 PointIndex)
{
	if (!CheckHasNativeAndValidPointIndex(ShapeContactRef, PointIndex, TEXT("Point Normal")))
	{
		return FVector::ZeroVector;
	}
	return ShapeContactRef.GetContactPoint(PointIndex).GetNormal();
}

float UAGX_ShapeContact_FL::GetPointArea(
	UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int32 PointIndex)
{
	if (!CheckHasNativeAndValidPointIndex(ShapeContactRef, PointIndex, TEXT("Point Area")))
	{
		return 0.0f;
	}
	return ShapeContactRef.GetContactPoint(PointIndex).GetArea();
}
