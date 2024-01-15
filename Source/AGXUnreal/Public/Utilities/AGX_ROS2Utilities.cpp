// Copyright 2024, Algoryx Simulation AB.

#include "AGX_ROS2Utilities.h"

// AGX Dynamics for Unreal includes.
#include "ROS2/AGX_ROS2Messages.h"

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
			const float Valf = FMath::Clamp((LColor.R + LColor.G + LColor.B) / 3.f, 0.f, 1.f) * MaxUint16f;
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
