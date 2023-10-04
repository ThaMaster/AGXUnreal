#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "ROS2/AGX_ROS2Messages.h"
#include "ROS2/AGX_ROS2Qos.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxROS2/MessageTypes.h>
#include <agxROS2/Qos.h>
#include "EndAGXIncludes.h"

//
// Qos
//

agxIO::ROS2::QOS_RELIABILITY Convert(EAGX_ROS2QosReliability Reliability)
{
	switch (Reliability)
	{
		case EAGX_ROS2QosReliability::ReliabilityDefault:
			return agxIO::ROS2::QOS_RELIABILITY::DEFAULT;
		case EAGX_ROS2QosReliability::BestEffort:
			return agxIO::ROS2::QOS_RELIABILITY::BEST_EFFORT;
		case EAGX_ROS2QosReliability::Reliable:
			return agxIO::ROS2::QOS_RELIABILITY::RELIABLE;
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("Unsupported Reliability QOS passed to Convert() function. Default Reliability QOS "
			 "will be used."));
	return agxIO::ROS2::QOS_RELIABILITY::DEFAULT;
}

agxIO::ROS2::QOS_DURABILITY Convert(EAGX_ROS2QosDurability Durability)
{
	switch (Durability)
	{
		case EAGX_ROS2QosDurability::DurabilityDefault:
			return agxIO::ROS2::QOS_DURABILITY::DEFAULT;
		case EAGX_ROS2QosDurability::Volatile:
			return agxIO::ROS2::QOS_DURABILITY::VOLATILE;
		case EAGX_ROS2QosDurability::TransientLocal:
			return agxIO::ROS2::QOS_DURABILITY::TRANSIENT_LOCAL;
		case EAGX_ROS2QosDurability::Transient:
			return agxIO::ROS2::QOS_DURABILITY::TRANSIENT;
		case EAGX_ROS2QosDurability::Persistent:
			return agxIO::ROS2::QOS_DURABILITY::PERSISTENT;
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("Unsupported Durability QOS passed to Convert() function. Default Durability QOS "
			 "will be used."));
	return agxIO::ROS2::QOS_DURABILITY::DEFAULT;
}

agxIO::ROS2::QOS_HISTORY Convert(EAGX_ROS2QosHistory History)
{
	switch (History)
	{
		case EAGX_ROS2QosHistory::HistoryDefault:
			return agxIO::ROS2::QOS_HISTORY::DEFAULT;
		case EAGX_ROS2QosHistory::KeepLastHistoryQos:
			return agxIO::ROS2::QOS_HISTORY::KEEP_LAST_HISTORY_QOS;
		case EAGX_ROS2QosHistory::KeepAllHistoryQos:
			return agxIO::ROS2::QOS_HISTORY::KEEP_ALL_HISTORY_QOS;
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("Unsupported History QOS passed to Convert() function. Default History QOS "
			 "will be used."));
	return agxIO::ROS2::QOS_HISTORY::DEFAULT;
}

agxIO::ROS2::QOS Convert(const FAGX_ROS2Qos& Qos)
{
	agxIO::ROS2::QOS QosROS2;
	QosROS2.reliability = Convert(Qos.Reliability);
	QosROS2.durability = Convert(Qos.Durability);
	QosROS2.history = Convert(Qos.History);
	QosROS2.historyDepth = Qos.HistoryDepth;

	return QosROS2;
}

//
// StdMsgs
//

agxIO::ROS2::stdMsgs::Float32 Convert(const FAGX_StdMsgsFloat32& InMsg)
{
	agxIO::ROS2::stdMsgs::Float32 Msg;
	Msg.data = InMsg.data;
	return Msg;
}

agxIO::ROS2::stdMsgs::Int32 Convert(const FAGX_StdMsgsInt32& InMsg)
{
	agxIO::ROS2::stdMsgs::Int32 Msg;
	Msg.data = InMsg.data;
	return Msg;
}
