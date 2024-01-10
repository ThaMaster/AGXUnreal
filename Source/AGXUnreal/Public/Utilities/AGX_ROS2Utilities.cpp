// Copyright 2023, Algoryx Simulation AB.

#include "AGX_ROS2Utilities.h"

// AGX Dynamics for Unreal includes.
#include "ROS2/AGX_ROS2Messages.h"

namespace AGX_ROS2Utilities_helpers
{
	template <typename PixelType, typename ChannelType, typename ChannelDoubleSizeType>
	FAGX_SensorMsgsImage Convert(
		const TArray<PixelType>& Image, float TimeStamp, const FIntPoint& Resolution,
		bool Grayscale, const FString& ChannelSize)
	{
		FAGX_SensorMsgsImage Msg;

		Msg.Header.Stamp.Sec = static_cast<int32>(TimeStamp);
		float Unused;
		Msg.Header.Stamp.Nanosec = static_cast<int32>(FMath::Modf(TimeStamp, &Unused)) * 1000000000;

		Msg.Height = static_cast<int64>(Resolution.Y);
		Msg.Width = static_cast<int64>(Resolution.X);

		if (Grayscale)
		{
			Msg.Step = Resolution.X * sizeof(ChannelType);
			Msg.Encoding = FString("mono") + ChannelSize;
			Msg.Data.Reserve(Image.Num());
			for (const auto& Color : Image)
			{
				const ChannelDoubleSizeType Sum = static_cast<ChannelDoubleSizeType>(Color.R) +
												  static_cast<ChannelDoubleSizeType>(Color.G) +
												  static_cast<ChannelDoubleSizeType>(Color.B);
				Msg.Data.Add(static_cast<ChannelType>(Sum / 3));
			}
		}
		else
		{
			Msg.Step = Resolution.X * sizeof(ChannelType) * 3;
			Msg.Encoding = FString("rgb") + ChannelSize;
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
}

FAGX_SensorMsgsImage FAGX_ROS2Utilities::Convert(
	const TArray<FColor>& Image, float TimeStamp, const FIntPoint& Resolution, bool Grayscale)
{
	return AGX_ROS2Utilities_helpers::Convert<FColor, uint8, uint16>(
		Image, TimeStamp, Resolution, Grayscale, "8");
}

FAGX_SensorMsgsImage FAGX_ROS2Utilities::Convert(
	const TArray<FFloat16Color>& Image, float TimeStamp, const FIntPoint& Resolution,
	bool Grayscale)
{
	return AGX_ROS2Utilities_helpers::Convert<FFloat16Color, uint16, uint32>(
		Image, TimeStamp, Resolution, Grayscale, "16");
}
