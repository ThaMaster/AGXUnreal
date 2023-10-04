#pragma once

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxROS2/Publisher.h>
#include <agxROS2/Subscriber.h>
#include "EndAGXIncludes.h"

struct FROS2Publisher
{
	virtual ~FROS2Publisher() = default;
};

struct FROS2Subscriber
{
	virtual ~FROS2Subscriber() = default;
};

//
// StdMsgs
//

struct FPublisherFloat32 : public FROS2Publisher
{
	FPublisherFloat32(agxIO::ROS2::stdMsgs::PublisherFloat32* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherFloat32> Native;
};

struct FPublisherInt32 : public FROS2Publisher
{
	FPublisherInt32(agxIO::ROS2::stdMsgs::PublisherInt32* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherInt32> Native;
};

struct FSubscriberFloat32 : public FROS2Subscriber
{
	FSubscriberFloat32(agxIO::ROS2::stdMsgs::SubscriberFloat32* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberFloat32> Native;
};

struct FSubscriberInt32 : public FROS2Subscriber
{
	FSubscriberInt32(agxIO::ROS2::stdMsgs::SubscriberInt32* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberInt32> Native;
};
