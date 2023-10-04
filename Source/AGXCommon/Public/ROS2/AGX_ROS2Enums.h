// Copyright 2023, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"

UENUM()
enum EAGX_ROS2MessageType
{
	Invalid,
	StdMsgsFloat32,
	StdMsgsInt32
};

UENUM()
enum EAGX_ROS2QosReliability
{
	ReliabilityDefault,
	BestEffort,
	Reliable
};

UENUM()
enum EAGX_ROS2QosDurability
{
	DurabilityDefault,
	Volatile,
	TransientLocal,
	Transient,
	Persistent
};

UENUM()
enum EAGX_ROS2QosHistory
{
	HistoryDefault,
	KeepLastHistoryQos,
	KeepAllHistoryQos
};

