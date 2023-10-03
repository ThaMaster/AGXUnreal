#pragma once

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxROS2/Publisher.h>
#include "EndAGXIncludes.h"

struct FROS2Publisher
{
	virtual ~FROS2Publisher() = default;
};

namespace StdMsgs
{
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
}
