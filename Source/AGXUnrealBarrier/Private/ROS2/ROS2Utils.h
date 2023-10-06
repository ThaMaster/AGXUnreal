#pragma once

namespace AGX_ROS2Utils
{
	/**
	 * These FreeContainsers functions are similar to the agxUtils::FreeContainerMemory dance we do
	 * for agx container types. That is, we need to free cross-dll-broundery container memory in a
	 * way that does not crash due to different allocators/deallocators being used in agx and
	 * Unreal.
	 * The rule here is: if a message has a container type that does heap allocation, we
	 * need to do this dance.
	 */

	template <typename MessageType>
	void FreeContainers(MessageType&)
	{
		// Base implementation, do nothing.
	}

	//
	// AgxMsgs
	//

	template <>
	void FreeContainers(agxIO::ROS2::agxMsgs::Any& Msg)
	{
		Msg.data.resize(0);
		Msg.data.shrink_to_fit();
	}

	template <>
	void FreeContainers(agxIO::ROS2::agxMsgs::AnySequence& Msg)
	{
		for (auto& D : Msg.data)
			FreeContainers(D);

		Msg.data.resize(0);
		Msg.data.shrink_to_fit();
	}

	//
	// StdMsgs
	//

	template <>
	void FreeContainers(agxIO::ROS2::stdMsgs::MultiArrayDimension& Msg)
	{
		Msg.label.resize(0);
		Msg.label.shrink_to_fit();
	}

	template <>
	void FreeContainers(agxIO::ROS2::stdMsgs::MultiArrayLayout& Msg)
	{
		for (auto& D : Msg.dim)
			FreeContainers(D);

		Msg.dim.resize(0);
		Msg.dim.shrink_to_fit();
	}

	template <>
	void FreeContainers(agxIO::ROS2::stdMsgs::ByteMultiArray& Msg)
	{
		FreeContainers(Msg.layout);
		Msg.data.resize(0);
		Msg.data.shrink_to_fit();
	}

	template <>
	void FreeContainers(agxIO::ROS2::stdMsgs::Float32MultiArray& Msg)
	{
		FreeContainers(Msg.layout);
		Msg.data.resize(0);
		Msg.data.shrink_to_fit();
	}

	template <>
	void FreeContainers(agxIO::ROS2::stdMsgs::Float64MultiArray& Msg)
	{
		FreeContainers(Msg.layout);
		Msg.data.resize(0);
		Msg.data.shrink_to_fit();
	}

	template <>
	void FreeContainers(agxIO::ROS2::stdMsgs::Int16MultiArray& Msg)
	{
		FreeContainers(Msg.layout);
		Msg.data.resize(0);
		Msg.data.shrink_to_fit();
	}

	template <>
	void FreeContainers(agxIO::ROS2::stdMsgs::Int32MultiArray& Msg)
	{
		FreeContainers(Msg.layout);
		Msg.data.resize(0);
		Msg.data.shrink_to_fit();
	}

	template <>
	void FreeContainers(agxIO::ROS2::stdMsgs::Int64MultiArray& Msg)
	{
		FreeContainers(Msg.layout);
		Msg.data.resize(0);
		Msg.data.shrink_to_fit();
	}

	template <>
	void FreeContainers(agxIO::ROS2::stdMsgs::Int8MultiArray& Msg)
	{
		FreeContainers(Msg.layout);
		Msg.data.resize(0);
		Msg.data.shrink_to_fit();
	}

	template <>
	void FreeContainers(agxIO::ROS2::stdMsgs::String& Msg)
	{
		Msg.data.resize(0);
		Msg.data.shrink_to_fit();
	}

	template <>
	void FreeContainers(agxIO::ROS2::stdMsgs::UInt16MultiArray& Msg)
	{
		FreeContainers(Msg.layout);
		Msg.data.resize(0);
		Msg.data.shrink_to_fit();
	}

	template <>
	void FreeContainers(agxIO::ROS2::stdMsgs::UInt32MultiArray& Msg)
	{
		FreeContainers(Msg.layout);
		Msg.data.resize(0);
		Msg.data.shrink_to_fit();
	}

	template <>
	void FreeContainers(agxIO::ROS2::stdMsgs::UInt64MultiArray& Msg)
	{
		FreeContainers(Msg.layout);
		Msg.data.resize(0);
		Msg.data.shrink_to_fit();
	}

	template <>
	void FreeContainers(agxIO::ROS2::stdMsgs::UInt8MultiArray& Msg)
	{
		FreeContainers(Msg.layout);
		Msg.data.resize(0);
		Msg.data.shrink_to_fit();
	}

	template <>
	void FreeContainers(agxIO::ROS2::stdMsgs::Header& Msg)
	{
		Msg.frame_id.resize(0);
		Msg.frame_id.shrink_to_fit();
	}

}
