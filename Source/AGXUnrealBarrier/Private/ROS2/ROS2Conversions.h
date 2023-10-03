#pragma once

// AGX Dynamics for Unreal includes.
#include "ROS2/AGX_ROS2Messages.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxROS2/MessageTypes.h>
#include "EndAGXIncludes.h"

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
