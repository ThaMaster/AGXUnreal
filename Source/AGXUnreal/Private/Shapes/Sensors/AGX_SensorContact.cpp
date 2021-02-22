#include "Shapes/Sensors/AGX_SensorContact.h"

// AGX Dynamics for Unreal includes.
#include "Shapes/AGX_ShapeComponent.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_LogCategory.h"

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

UAGX_ShapeComponent* UAGX_SensorContact_FL::GetFirstShape(UPARAM(ref)
															  FAGX_SensorContact& SensorContactRef)
{
	return GetFromGuid<UAGX_ShapeComponent>(SensorContactRef.Data.FirstShapeGuid);
}

UAGX_ShapeComponent* UAGX_SensorContact_FL::GetSecondShape(UPARAM(ref)
															   FAGX_SensorContact& SensorContactRef)
{
	return GetFromGuid<UAGX_ShapeComponent>(SensorContactRef.Data.SecondShapeGuid);
}

UAGX_RigidBodyComponent* UAGX_SensorContact_FL::GetFirstBody(
	UPARAM(ref) FAGX_SensorContact& SensorContactRef)
{
	return GetFromGuid<UAGX_RigidBodyComponent>(SensorContactRef.Data.FirstBodyGuid);
}

UAGX_RigidBodyComponent* UAGX_SensorContact_FL::GetSecondBody(
	UPARAM(ref) FAGX_SensorContact& SensorContactRef)
{
	return GetFromGuid<UAGX_RigidBodyComponent>(SensorContactRef.Data.SecondBodyGuid);
}

int UAGX_SensorContact_FL::GetNumPoints(UPARAM(ref) FAGX_SensorContact& SensorContactRef)
{
	// UFUNCTIONS does not support size_t or uint32, so the best we can do is to give a warning
	// if the numbe of points are above the maximum value for an int.
	size_t NumPoints = SensorContactRef.Data.Points.Num();
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

FVector UAGX_SensorContact_FL::GetPointPosition(
	UPARAM(ref) FAGX_SensorContact& SensorContactRef, int PointIndex)
{
	if (PointIndex >= SensorContactRef.Data.Points.Num())
	{
		UE_LOG(LogAGX, Error, TEXT("GetPointPosition: tried to access point index out of bounds."));
		return FVector::ZeroVector;
	}

	return SensorContactRef.Data.Points[PointIndex].Position;
}

FVector UAGX_SensorContact_FL::GetPointForce(
	UPARAM(ref) FAGX_SensorContact& SensorContactRef, int PointIndex)
{
	if (PointIndex >= SensorContactRef.Data.Points.Num())
	{
		UE_LOG(LogAGX, Error, TEXT("GetPointForce: tried to access point index out of bounds."));
		return FVector::ZeroVector;
	}

	return SensorContactRef.Data.Points[PointIndex].Force;
}

FVector UAGX_SensorContact_FL::GetPointNormalForce(
	UPARAM(ref) FAGX_SensorContact& SensorContactRef, int PointIndex)
{
	if (PointIndex >= SensorContactRef.Data.Points.Num())
	{
		UE_LOG(
			LogAGX, Error, TEXT("GetPointNormalForce: tried to access point index out of bounds."));
		return FVector::ZeroVector;
	}

	return SensorContactRef.Data.Points[PointIndex].NomalForce;
}

FVector UAGX_SensorContact_FL::GetPointNormal(
	UPARAM(ref) FAGX_SensorContact& SensorContactRef, int PointIndex)
{
	if (PointIndex >= SensorContactRef.Data.Points.Num())
	{
		UE_LOG(LogAGX, Error, TEXT("GetPointNormal: tried to access point index out of bounds."));
		return FVector::ZeroVector;
	}

	return SensorContactRef.Data.Points[PointIndex].Normal;
}

float UAGX_SensorContact_FL::GetPointDepth(
	UPARAM(ref) FAGX_SensorContact& SensorContactRef, int PointIndex)
{
	if (PointIndex >= SensorContactRef.Data.Points.Num())
	{
		UE_LOG(LogAGX, Error, TEXT("GetPointDepth: tried to access point index out of bounds."));
		return 0.f;
	}

	return SensorContactRef.Data.Points[PointIndex].Depth;
}

float UAGX_SensorContact_FL::GetPointArea(
	UPARAM(ref) FAGX_SensorContact& SensorContactRef, int PointIndex)
{
	if (PointIndex >= SensorContactRef.Data.Points.Num())
	{
		UE_LOG(LogAGX, Error, TEXT("GetPointArea: tried to access point index out of bounds."));
		return 0.f;
	}

	return SensorContactRef.Data.Points[PointIndex].Area;
}
