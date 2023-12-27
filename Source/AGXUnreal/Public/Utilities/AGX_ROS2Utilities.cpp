// Copyright 2023, Algoryx Simulation AB.

#include "AGX_ROS2Utilities.h"

// AGX Dynamics for Unreal includes.
#include "ROS2/AGX_ROS2Messages.h"

FAGX_SensorMsgsImage FAGX_ROS2Utilities::Convert(
	const TArray<FColor>& Image, float TimeStamp, const FIntPoint& Resolution, bool Grayscale)
{
	FAGX_SensorMsgsImage Msg;

	Msg.Header.Stamp.Sec = static_cast<int32>(TimeStamp);
	float Unused;
	Msg.Header.Stamp.Nanosec = static_cast<int32>(FMath::Modf(TimeStamp, &Unused)) * 1000000000;

	Msg.Height = static_cast<int64>(Resolution.Y);
	Msg.Width = static_cast<int64>(Resolution.X);

	if (Grayscale)
	{
		Msg.Step = Resolution.X * sizeof(uint8);
		Msg.Encoding = TEXT("mono8");
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
		Msg.Step = Resolution.X * sizeof(uint8) * 3;
		Msg.Encoding = TEXT("rgb8");
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
