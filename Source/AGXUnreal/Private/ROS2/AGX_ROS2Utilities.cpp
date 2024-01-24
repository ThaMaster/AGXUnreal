// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/AGX_ROS2Utilities.h"

// AGX Dynamics for Unreal includes.
#include "ROS2/AGX_ROS2Messages.h"
#include "Sensors/AGX_LidarScanPoint.h"

// Standard library includes.
#include <limits>

namespace AGX_ROS2Utilities_helpers
{
	template <typename PixelType, typename OutputChannelType>
	FAGX_SensorMsgsImage SetAllExceptData(
		const TArray<PixelType>& Image, float TimeStamp, const FIntPoint& Resolution,
		bool Grayscale, const FString& ChannelSize)
	{
		FAGX_SensorMsgsImage Msg;

		Msg.IsBigendian = 0;
		Msg.Header.Stamp.Sec = static_cast<int32>(TimeStamp);
		float Unused;
		Msg.Header.Stamp.Nanosec = static_cast<int32>(FMath::Modf(TimeStamp, &Unused)) * 1000000000;

		Msg.Height = static_cast<int64>(Resolution.Y);
		Msg.Width = static_cast<int64>(Resolution.X);

		if (Grayscale)
		{
			Msg.Step = Resolution.X * sizeof(OutputChannelType);
			Msg.Encoding = FString("mono") + ChannelSize;
		}
		else
		{
			Msg.Step = Resolution.X * sizeof(OutputChannelType) * 3;
			Msg.Encoding = FString("rgb") + ChannelSize;
		}

		return Msg;
	}

	FAGX_SensorMsgsPointField MakePointFieldField(
		const FString& Name, int64 Offset, uint8 Datatype, int64 Count)
	{
		FAGX_SensorMsgsPointField Field;
		Field.Name = Name;
		Field.Offset = Offset;
		Field.Datatype = Datatype;
		Field.Count = Count;
		return Field;
	};

	void AppendDoubleToUint8Array(double Val, TArray<uint8>& OutData)
	{
		static_assert(sizeof(uint64) == sizeof(double));
		uint64 Bits = *reinterpret_cast<uint64*>(&Val);
		for (int i = 0; i < sizeof(double); i++)
		{
			OutData.Add(static_cast<uint8_t>(Bits & 0xFF));
			Bits >>= 8;
		}
	};

	auto AppendUint32ToUint8Array(uint32 Val, TArray<uint8>& OutData)
	{
		for (int i = 0; i < sizeof(uint32); i++)
		{
			OutData.Add(static_cast<uint8_t>(Val & 0xFF));
			Val >>= 8;
		}
	};
}

FAGX_SensorMsgsImage FAGX_ROS2Utilities::Convert(
	const TArray<FColor>& Image, float TimeStamp, const FIntPoint& Resolution, bool Grayscale)
{
	static_assert(sizeof(FColor::R) == sizeof(uint8));
	FAGX_SensorMsgsImage Msg = AGX_ROS2Utilities_helpers::SetAllExceptData<FColor, uint8>(
		Image, TimeStamp, Resolution, Grayscale, "8");

	if (Grayscale)
	{
		Msg.Data.Reserve(Image.Num());
		for (const auto& Color : Image)
		{
			const uint16 Sum = static_cast<uint16>(Color.R) + static_cast<uint16>(Color.G) +
							   static_cast<uint16>(Color.B);
			Msg.Data.Add(static_cast<uint8>(Sum / 3));
		}
	}
	else
	{
		Msg.Data.Reserve(Image.Num() * 3);
		for (const auto& Color : Image)
		{
			Msg.Data.Add(Color.R);
			Msg.Data.Add(Color.G);
			Msg.Data.Add(Color.B);
		}
	}

	return Msg;
}

FAGX_SensorMsgsImage FAGX_ROS2Utilities::Convert(
	const TArray<FFloat16Color>& Image, float TimeStamp, const FIntPoint& Resolution,
	bool Grayscale)
{
	FAGX_SensorMsgsImage Msg = AGX_ROS2Utilities_helpers::SetAllExceptData<FFloat16Color, uint16>(
		Image, TimeStamp, Resolution, Grayscale, "16");

	static constexpr float MaxUint16f = static_cast<float>(std::numeric_limits<uint16>::max());
	if (Grayscale)
	{
		Msg.Data.Reserve(Image.Num() * 2);
		for (const auto& Color : Image)
		{
			const FLinearColor LColor = Color.GetFloats();

			// Transform from [0..1] to uint16 range.
			const float Valf =
				FMath::Clamp((LColor.R + LColor.G + LColor.B) / 3.f, 0.f, 1.f) * MaxUint16f;
			const uint16 Val = static_cast<uint16>(Valf);

			Msg.Data.Add(static_cast<uint8>(Val & 0xFF)); // Low bits.
			Msg.Data.Add(static_cast<uint8>((Val >> 8) & 0xFF)); // High bits.
		}
	}
	else
	{
		Msg.Data.Reserve(Image.Num() * 2 * 3);
		for (const auto& Color : Image)
		{
			const FLinearColor LColor = Color.GetFloats();

			// Transform from [0..1] to uint16 range.
			const uint16 Rgb[3] = {
				static_cast<uint16>(FMath::Clamp(LColor.R, 0.f, 1.f) * MaxUint16f),
				static_cast<uint16>(FMath::Clamp(LColor.G, 0.f, 1.f) * MaxUint16f),
				static_cast<uint16>(FMath::Clamp(LColor.B, 0.f, 1.f) * MaxUint16f)};

			for (const uint16 C : Rgb)
			{
				Msg.Data.Add(static_cast<uint8>(C & 0xFF)); // Low bits.
				Msg.Data.Add(static_cast<uint8>((C >> 8) & 0xFF)); // High bits.
			}
		}
	}

	return Msg;
}

FAGX_SensorMsgsPointCloud2 FAGX_ROS2Utilities::ConvertXYZ(
	const TArray<FAGX_LidarScanPoint>& Points, int32 Width, int32 Height)
{
	using namespace AGX_ROS2Utilities_helpers;
	FAGX_SensorMsgsPointCloud2 Msg;

	const int32 FirstValidIndex =
		Points.IndexOfByPredicate([](const FAGX_LidarScanPoint& P) { return P.bIsValid; });
	if (FirstValidIndex == INDEX_NONE)
		return Msg;

	Msg.Header.Stamp.Sec = static_cast<int32>(Points[FirstValidIndex].TimeStamp);
	float Unused;
	Msg.Header.Stamp.Nanosec =
		static_cast<int32>(FMath::Modf(Points[FirstValidIndex].TimeStamp, &Unused)) * 1e9;

	Msg.Fields.Add(MakePointFieldField("x", 0, 8, 1));
	Msg.Fields.Add(MakePointFieldField("y", 8, 8, 1));
	Msg.Fields.Add(MakePointFieldField("z", 16, 8, 1));
	Msg.Fields.Add(MakePointFieldField("intensity", 24, 8, 1));

	Msg.IsBigendian = false;
	Msg.PointStep = 32;
	Msg.RowStep = Width * Msg.PointStep;
	Msg.IsDense = true;

	Msg.Data.Reserve(Points.Num() * Msg.PointStep);
	for (int32 i = FirstValidIndex; i < Points.Num(); i++)
	{
		if (!Points[i].bIsValid)
			continue;

		AppendDoubleToUint8Array(Points[i].Position.X, Msg.Data);
		AppendDoubleToUint8Array(Points[i].Position.Y, Msg.Data);
		AppendDoubleToUint8Array(Points[i].Position.Z, Msg.Data);
		AppendDoubleToUint8Array(Points[i].Intensity, Msg.Data);
	}

	return Msg;
}

FAGX_SensorMsgsPointCloud2 FAGX_ROS2Utilities::ConvertAnglesTOF(
	const TArray<FAGX_LidarScanPoint>& Points)
{
	using namespace AGX_ROS2Utilities_helpers;
	FAGX_SensorMsgsPointCloud2 Msg;

	const int32 FirstValidIndex =
		Points.IndexOfByPredicate([](const FAGX_LidarScanPoint& P) { return P.bIsValid; });
	if (FirstValidIndex == INDEX_NONE)
		return Msg;

	Msg.Header.Stamp.Sec = static_cast<int32>(Points[FirstValidIndex].TimeStamp);
	float Unused;
	Msg.Header.Stamp.Nanosec =
		static_cast<int32>(FMath::Modf(Points[FirstValidIndex].TimeStamp, &Unused)) * 1e9;

	Msg.Fields.Add(MakePointFieldField("angle_x", 0, 8, 1));
	Msg.Fields.Add(MakePointFieldField("angle_y", 8, 8, 1));
	Msg.Fields.Add(MakePointFieldField("tof", 16, 6, 1));
	Msg.Fields.Add(MakePointFieldField("intensity", 24, 8, 1));

	Msg.IsBigendian = false;
	Msg.PointStep = 28;
	Msg.RowStep = Points.Num() * Msg.PointStep;
	Msg.IsDense = true;

	// TimePiko = DistanceMeters / C * 1.0e12 * 2.
	// Where C is the speed of light. The factor 2 is because the ray travels to the object and back
	// again.
	// We collect the constants and get TimePiko = DistanceMeters * K.
	static constexpr double K = 2.0 * 1.0e12 / 299792458.0;

	Msg.Data.Reserve(Points.Num() * Msg.PointStep);
	for (int32 i = FirstValidIndex; i < Points.Num(); i++)
	{
		if (!Points[i].bIsValid)
			continue;

		const double AngleX = FMath::Atan2(Points[i].Position.Y, Points[i].Position.X);
		const double AngleY = FMath::Atan2(
			Points[i].Position.Z,
			FMath::Sqrt(FMath::Pow(Points[i].Position.X, 2) + FMath::Pow(Points[i].Position.Y, 2)));

		const double Distance = 0.01 * Points[i].Position.Length(); // In meters.
		const double TimePikoSecondsd = Distance * K;
		uint32 TimePikoSeconds = static_cast<uint32>(TimePikoSecondsd);
		if (TimePikoSecondsd > std::numeric_limits<uint32>::max())
		{
			// This means we have a measurement that is more than 643799 meters away which is
			// unlikely for a Lidar Sensor. We could use uint64 here, but that's not part of the
			// specification for sensor_msgs::PointField, so we stick with the supported uint32
			// until we need to represent larger values than this.
			UE_LOG(
				LogAGX, Warning,
				TEXT("ConvertAnglesTOF got time in pikoseconds: %f which is too large to store in "
					 "an uint32. std::numeric_limits<uint32>::max() is used instead."),
				TimePikoSecondsd);
			TimePikoSeconds = std::numeric_limits<uint32>::max();
		}

		// Append data to Msg.Data
		AppendDoubleToUint8Array(AngleX, Msg.Data);
		AppendDoubleToUint8Array(AngleY, Msg.Data);
		AppendUint32ToUint8Array(TimePikoSeconds, Msg.Data);
		AppendDoubleToUint8Array(Points[i].Intensity, Msg.Data);
	}

	return Msg;
}
