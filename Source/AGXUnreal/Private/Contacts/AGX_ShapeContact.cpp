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

namespace
{
	bool checkHasNative(const FAGX_ShapeContact& ShapeContact, const TCHAR* AttributeName)
	{
		if (ShapeContact.HasNative())
		{
			return true;
		}

		UE_LOG(
			LogAGX, Error,
			TEXT("Trying to get %s from a ShapeContact that doesn't have a native AGX Dynamics "
				 "representation"));
		return false;
	}
}

FRigidBodyBarrier FAGX_ShapeContact::GetBody1() const
{
	if (!checkHasNative(*this, TEXT("First Body")))
	{
		return FRigidBodyBarrier();
	}
	return Barrier.GetBody1();
}

FRigidBodyBarrier FAGX_ShapeContact::GetBody2() const
{
	if (!checkHasNative(*this, TEXT("Second Body")))
	{
		return FRigidBodyBarrier();
	}
	return Barrier.GetBody2();
}

FEmptyShapeBarrier FAGX_ShapeContact::GetShape1() const
{
	if (!checkHasNative(*this, TEXT("First Shape")))
	{
		return FEmptyShapeBarrier();
	}
	return Barrier.GetShape1();
}

FEmptyShapeBarrier FAGX_ShapeContact::GetShape2() const
{
	if (!checkHasNative(*this, TEXT("Second Shape")))
	{
		return FEmptyShapeBarrier();
	}
	return Barrier.GetShape2();
}

int32 FAGX_ShapeContact::GetNumContactPoints() const
{
	if (!checkHasNative(*this, TEXT("Num Contact Points")))
	{
		return 0;
	}
	return Barrier.GetNumContactPoints();
}

TArray<FAGX_ContactPoint> FAGX_ShapeContact::GetContactPoints() const
{
	if (!checkHasNative(*this, TEXT("Contact Points")))
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

FAGX_ContactPoint FAGX_ShapeContact::GetContactPoint(int Index) const
{
	if (!checkHasNative(*this, TEXT("Contact Point")))
	{
		return FAGX_ContactPoint();
	}
	return Barrier.GetContactPoint(Index);
}

int UAGX_ShapeContact_FL::GetNumContactPoints(UPARAM(ref) FAGX_ShapeContact& ShapeContactRef)
{
	if (!checkHasNative(ShapeContactRef, TEXT("Num Contact Points")))
	{
		return 0;
	}
	return ShapeContactRef.GetNumContactPoints();
}

float UAGX_ShapeContact_FL::GetPointDepth(
	UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int PointIndex)
{
	if (!checkHasNative(ShapeContactRef, TEXT("Point Depth")))
	{
		return 0.0f;
	}
	if (PointIndex >= ShapeContactRef.GetNumContactPoints())
	{
		/// @todo Figure out how to get the shape/body names here.
		UE_LOG(
			LogAGX, Error,
			TEXT("GetPointDepth: Tried to access point index out of bounds in ShapeContact between "
				 "'%s' and '%s'."),
			TEXT("TODO!"), TEXT("TODO!"));
		return 0.0f;
	}

	FAGX_ContactPoint Point = ShapeContactRef.GetContactPoint(PointIndex);
	if (!Point.HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("GetPointDepth: Could not get contact point %d from ShapeContact between '%s' and "
				 "'%s'."),
			TEXT("TODO!"), TEXT("TODO!"));
		return 0.0f;
	}

	return Point.GetDepth();
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
	if (!checkHasNative(ShapeContactRef, TEXT("First Shape")))
	{
		return nullptr;
	}
	return GetFromGuid<UAGX_ShapeComponent>(ShapeContactRef.GetShape1().GetGeometryGuid());
}

UAGX_ShapeComponent* UAGX_ShapeContact_FL::GetSecondShape(UPARAM(ref)
															  FAGX_ShapeContact& ShapeContactRef)
{
	if (!checkHasNative(ShapeContactRef, TEXT("Second Shape")))
	{
		return nullptr;
	}
	return GetFromGuid<UAGX_ShapeComponent>(ShapeContactRef.GetShape2().GetGeometryGuid());
}

UAGX_RigidBodyComponent* UAGX_ShapeContact_FL::GetFirstBody(UPARAM(ref)
																FAGX_ShapeContact& ShapeContactRef)
{
	if (!checkHasNative(ShapeContactRef, TEXT("First Body")))
	{
		return nullptr;
	}
	return GetFromGuid<UAGX_RigidBodyComponent>(ShapeContactRef.GetBody1().GetGuid());
}

UAGX_RigidBodyComponent* UAGX_ShapeContact_FL::GetSecondBody(UPARAM(ref)
																 FAGX_ShapeContact& ShapeContactRef)
{
	if (!checkHasNative(ShapeContactRef, TEXT("Second Body")))
	{
		return nullptr;
	}
	return GetFromGuid<UAGX_RigidBodyComponent>(ShapeContactRef.GetBody2().GetGuid());
}

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
			TEXT("Trying to get %s from contact point at index %d in ShapeContact that only has %d "
				 "contact points."),
			AttributeName, PointIndex, NumContactPoints);
		return false;
	}
}

FVector UAGX_ShapeContact_FL::GetPointLocation(
	UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int PointIndex)
{
	if (!checkHasNative(ShapeContactRef, TEXT("GetPointLocation")))
	{
		return FVector::ZeroVector;
	}
	if (!IsValidPointIndex(ShapeContactRef, PointIndex, TEXT("Location")))
	{
		return FVector::ZeroVector;
	}
	return ShapeContactRef.GetContactPoint(PointIndex).GetLocation();
}

FVector UAGX_ShapeContact_FL::GetPointForce(
	UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int PointIndex)
{
	if (!checkHasNative(ShapeContactRef, TEXT("Point Force")))
	{
		return FVector::ZeroVector;
	}
	if (!IsValidPointIndex(ShapeContactRef, PointIndex, TEXT("Force")))
	{
		return FVector::ZeroVector;
	}
	return ShapeContactRef.GetContactPoint(PointIndex).GetForce();
}

FVector UAGX_ShapeContact_FL::GetPointNormalForce(
	UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int PointIndex)
{
	if (!checkHasNative(ShapeContactRef, TEXT("Point Normal Force")))

	{
		return FVector::ZeroVector;
	}
	if (!IsValidPointIndex(ShapeContactRef, PointIndex, TEXT("Normal Force")))
	{
		return FVector::ZeroVector;
	}
	return ShapeContactRef.GetContactPoint(PointIndex).GetNormalForce();
}

FVector UAGX_ShapeContact_FL::GetPointNormal(
	UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int PointIndex)
{
	if (!checkHasNative(ShapeContactRef, TEXT("Point Normal")))
	{
		return FVector::ZeroVector;
	}
	if (!IsValidPointIndex(ShapeContactRef, PointIndex, TEXT("Normal")))
	{
		return FVector::ZeroVector;
	}
	return ShapeContactRef.GetContactPoint(PointIndex).GetNormal();
}

float UAGX_ShapeContact_FL::GetPointArea(
	UPARAM(ref) FAGX_ShapeContact& ShapeContactRef, int PointIndex)
{
	if (!checkHasNative(ShapeContactRef, TEXT("Point Area")))
	{
		return 0.0f;
	}
	if (!IsValidPointIndex(ShapeContactRef, PointIndex, TEXT("Area")))
	{
		return 0.0f;
	}
	return ShapeContactRef.GetContactPoint(PointIndex).GetArea();
}
