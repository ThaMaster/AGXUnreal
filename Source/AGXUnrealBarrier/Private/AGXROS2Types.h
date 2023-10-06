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
// AgxMsgs
//

struct FPublisherAny : public FROS2Publisher
{
	FPublisherAny(agxIO::ROS2::agxMsgs::PublisherAny* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::agxMsgs::PublisherAny> Native;
};

struct FPublisherAnySequence : public FROS2Publisher
{
	FPublisherAnySequence(agxIO::ROS2::agxMsgs::PublisherAnySequence* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::agxMsgs::PublisherAnySequence> Native;
};

struct FSubscriberAny : public FROS2Subscriber
{
	FSubscriberAny(agxIO::ROS2::agxMsgs::SubscriberAny* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::agxMsgs::SubscriberAny> Native;
};

struct FSubscriberAnySequence : public FROS2Subscriber
{
	FSubscriberAnySequence(agxIO::ROS2::agxMsgs::SubscriberAnySequence* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::agxMsgs::SubscriberAnySequence> Native;
};

//
// BuiltinInterfaces
//

struct FPublisherTime : public FROS2Publisher
{
	FPublisherTime(agxIO::ROS2::builtinInterfaces::PublisherTime* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::builtinInterfaces::PublisherTime> Native;
};

struct FPublisherDuration : public FROS2Publisher
{
	FPublisherDuration(agxIO::ROS2::builtinInterfaces::PublisherDuration* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::builtinInterfaces::PublisherDuration> Native;
};

struct FSubscriberTime : public FROS2Subscriber
{
	FSubscriberTime(agxIO::ROS2::builtinInterfaces::SubscriberTime* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::builtinInterfaces::SubscriberTime> Native;
};

struct FSubscriberDuration : public FROS2Subscriber
{
	FSubscriberDuration(agxIO::ROS2::builtinInterfaces::SubscriberDuration* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::builtinInterfaces::SubscriberDuration> Native;
};

//
// RosgraphMsgs
//

struct FPublisherClock : public FROS2Publisher
{
	FPublisherClock(agxIO::ROS2::rosgraphMsgs::PublisherClock* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::rosgraphMsgs::PublisherClock> Native;
};

struct FSubscriberClock : public FROS2Subscriber
{
	FSubscriberClock(agxIO::ROS2::rosgraphMsgs::SubscriberClock* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::rosgraphMsgs::SubscriberClock> Native;
};

//
// StdMsgs
//

struct FPublisherBool : public FROS2Publisher
{
	FPublisherBool(agxIO::ROS2::stdMsgs::PublisherBool* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherBool> Native;
};

struct FPublisherByte : public FROS2Publisher
{
	FPublisherByte(agxIO::ROS2::stdMsgs::PublisherByte* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherByte> Native;
};

struct FPublisherByteMultiArray : public FROS2Publisher
{
	FPublisherByteMultiArray(agxIO::ROS2::stdMsgs::PublisherByteMultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherByteMultiArray> Native;
};

struct FPublisherChar : public FROS2Publisher
{
	FPublisherChar(agxIO::ROS2::stdMsgs::PublisherChar* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherChar> Native;
};

struct FPublisherColorRGBA : public FROS2Publisher
{
	FPublisherColorRGBA(agxIO::ROS2::stdMsgs::PublisherColorRGBA* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherColorRGBA> Native;
};

struct FPublisherEmpty : public FROS2Publisher
{
	FPublisherEmpty(agxIO::ROS2::stdMsgs::PublisherEmpty* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherEmpty> Native;
};

struct FPublisherFloat32 : public FROS2Publisher
{
	FPublisherFloat32(agxIO::ROS2::stdMsgs::PublisherFloat32* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherFloat32> Native;
};

struct FPublisherFloat32MultiArray : public FROS2Publisher
{
	FPublisherFloat32MultiArray(agxIO::ROS2::stdMsgs::PublisherFloat32MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherFloat32MultiArray> Native;
};

struct FPublisherFloat64 : public FROS2Publisher
{
	FPublisherFloat64(agxIO::ROS2::stdMsgs::PublisherFloat64* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherFloat64> Native;
};

struct FPublisherFloat64MultiArray : public FROS2Publisher
{
	FPublisherFloat64MultiArray(agxIO::ROS2::stdMsgs::PublisherFloat64MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherFloat64MultiArray> Native;
};

struct FPublisherInt16 : public FROS2Publisher
{
	FPublisherInt16(agxIO::ROS2::stdMsgs::PublisherInt16* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherInt16> Native;
};

struct FPublisherInt16MultiArray : public FROS2Publisher
{
	FPublisherInt16MultiArray(agxIO::ROS2::stdMsgs::PublisherInt16MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherInt16MultiArray> Native;
};

struct FPublisherInt32 : public FROS2Publisher
{
	FPublisherInt32(agxIO::ROS2::stdMsgs::PublisherInt32* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherInt32> Native;
};

struct FPublisherInt32MultiArray : public FROS2Publisher
{
	FPublisherInt32MultiArray(agxIO::ROS2::stdMsgs::PublisherInt32MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherInt32MultiArray> Native;
};

struct FPublisherInt64 : public FROS2Publisher
{
	FPublisherInt64(agxIO::ROS2::stdMsgs::PublisherInt64* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherInt64> Native;
};

struct FPublisherInt64MultiArray : public FROS2Publisher
{
	FPublisherInt64MultiArray(agxIO::ROS2::stdMsgs::PublisherInt64MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherInt64MultiArray> Native;
};

struct FPublisherInt8 : public FROS2Publisher
{
	FPublisherInt8(agxIO::ROS2::stdMsgs::PublisherInt8* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherInt8> Native;
};

struct FPublisherInt8MultiArray : public FROS2Publisher
{
	FPublisherInt8MultiArray(agxIO::ROS2::stdMsgs::PublisherInt8MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherInt8MultiArray> Native;
};

struct FPublisherString : public FROS2Publisher
{
	FPublisherString(agxIO::ROS2::stdMsgs::PublisherString* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherString> Native;
};

struct FPublisherUInt16 : public FROS2Publisher
{
	FPublisherUInt16(agxIO::ROS2::stdMsgs::PublisherUInt16* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherUInt16> Native;
};

struct FPublisherUInt16MultiArray : public FROS2Publisher
{
	FPublisherUInt16MultiArray(agxIO::ROS2::stdMsgs::PublisherUInt16MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherUInt16MultiArray> Native;
};

struct FPublisherUInt32 : public FROS2Publisher
{
	FPublisherUInt32(agxIO::ROS2::stdMsgs::PublisherUInt32* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherUInt32> Native;
};

struct FPublisherUInt32MultiArray : public FROS2Publisher
{
	FPublisherUInt32MultiArray(agxIO::ROS2::stdMsgs::PublisherUInt32MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherUInt32MultiArray> Native;
};

struct FPublisherUInt64 : public FROS2Publisher
{
	FPublisherUInt64(agxIO::ROS2::stdMsgs::PublisherUInt64* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherUInt64> Native;
};

struct FPublisherUInt64MultiArray : public FROS2Publisher
{
	FPublisherUInt64MultiArray(agxIO::ROS2::stdMsgs::PublisherUInt64MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherUInt64MultiArray> Native;
};

struct FPublisherUInt8 : public FROS2Publisher
{
	FPublisherUInt8(agxIO::ROS2::stdMsgs::PublisherUInt8* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherUInt8> Native;
};

struct FPublisherUInt8MultiArray : public FROS2Publisher
{
	FPublisherUInt8MultiArray(agxIO::ROS2::stdMsgs::PublisherUInt8MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherUInt8MultiArray> Native;
};

struct FPublisherHeader : public FROS2Publisher
{
	FPublisherHeader(agxIO::ROS2::stdMsgs::PublisherHeader* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::PublisherHeader> Native;
};

struct FSubscriberBool : public FROS2Subscriber
{
	FSubscriberBool(agxIO::ROS2::stdMsgs::SubscriberBool* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberBool> Native;
};

struct FSubscriberByte : public FROS2Subscriber
{
	FSubscriberByte(agxIO::ROS2::stdMsgs::SubscriberByte* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberByte> Native;
};

struct FSubscriberByteMultiArray : public FROS2Subscriber
{
	FSubscriberByteMultiArray(agxIO::ROS2::stdMsgs::SubscriberByteMultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberByteMultiArray> Native;
};

struct FSubscriberChar : public FROS2Subscriber
{
	FSubscriberChar(agxIO::ROS2::stdMsgs::SubscriberChar* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberChar> Native;
};

struct FSubscriberColorRGBA : public FROS2Subscriber
{
	FSubscriberColorRGBA(agxIO::ROS2::stdMsgs::SubscriberColorRGBA* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberColorRGBA> Native;
};

struct FSubscriberEmpty : public FROS2Subscriber
{
	FSubscriberEmpty(agxIO::ROS2::stdMsgs::SubscriberEmpty* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberEmpty> Native;
};

struct FSubscriberFloat32 : public FROS2Subscriber
{
	FSubscriberFloat32(agxIO::ROS2::stdMsgs::SubscriberFloat32* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberFloat32> Native;
};

struct FSubscriberFloat32MultiArray : public FROS2Subscriber
{
	FSubscriberFloat32MultiArray(agxIO::ROS2::stdMsgs::SubscriberFloat32MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberFloat32MultiArray> Native;
};

struct FSubscriberFloat64 : public FROS2Subscriber
{
	FSubscriberFloat64(agxIO::ROS2::stdMsgs::SubscriberFloat64* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberFloat64> Native;
};

struct FSubscriberFloat64MultiArray : public FROS2Subscriber
{
	FSubscriberFloat64MultiArray(agxIO::ROS2::stdMsgs::SubscriberFloat64MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberFloat64MultiArray> Native;
};

struct FSubscriberInt16 : public FROS2Subscriber
{
	FSubscriberInt16(agxIO::ROS2::stdMsgs::SubscriberInt16* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberInt16> Native;
};

struct FSubscriberInt16MultiArray : public FROS2Subscriber
{
	FSubscriberInt16MultiArray(agxIO::ROS2::stdMsgs::SubscriberInt16MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberInt16MultiArray> Native;
};

struct FSubscriberInt32 : public FROS2Subscriber
{
	FSubscriberInt32(agxIO::ROS2::stdMsgs::SubscriberInt32* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberInt32> Native;
};

struct FSubscriberInt32MultiArray : public FROS2Subscriber
{
	FSubscriberInt32MultiArray(agxIO::ROS2::stdMsgs::SubscriberInt32MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberInt32MultiArray> Native;
};

struct FSubscriberInt64 : public FROS2Subscriber
{
	FSubscriberInt64(agxIO::ROS2::stdMsgs::SubscriberInt64* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberInt64> Native;
};

struct FSubscriberInt64MultiArray : public FROS2Subscriber
{
	FSubscriberInt64MultiArray(agxIO::ROS2::stdMsgs::SubscriberInt64MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberInt64MultiArray> Native;
};

struct FSubscriberInt8 : public FROS2Subscriber
{
	FSubscriberInt8(agxIO::ROS2::stdMsgs::SubscriberInt8* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberInt8> Native;
};

struct FSubscriberInt8MultiArray : public FROS2Subscriber
{
	FSubscriberInt8MultiArray(agxIO::ROS2::stdMsgs::SubscriberInt8MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberInt8MultiArray> Native;
};

struct FSubscriberString : public FROS2Subscriber
{
	FSubscriberString(agxIO::ROS2::stdMsgs::SubscriberString* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberString> Native;
};

struct FSubscriberUInt16 : public FROS2Subscriber
{
	FSubscriberUInt16(agxIO::ROS2::stdMsgs::SubscriberUInt16* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberUInt16> Native;
};

struct FSubscriberUInt16MultiArray : public FROS2Subscriber
{
	FSubscriberUInt16MultiArray(agxIO::ROS2::stdMsgs::SubscriberUInt16MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberUInt16MultiArray> Native;
};

struct FSubscriberUInt32 : public FROS2Subscriber
{
	FSubscriberUInt32(agxIO::ROS2::stdMsgs::SubscriberUInt32* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberUInt32> Native;
};

struct FSubscriberUInt32MultiArray : public FROS2Subscriber
{
	FSubscriberUInt32MultiArray(agxIO::ROS2::stdMsgs::SubscriberUInt32MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberUInt32MultiArray> Native;
};

struct FSubscriberUInt64 : public FROS2Subscriber
{
	FSubscriberUInt64(agxIO::ROS2::stdMsgs::SubscriberUInt64* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberUInt64> Native;
};

struct FSubscriberUInt64MultiArray : public FROS2Subscriber
{
	FSubscriberUInt64MultiArray(agxIO::ROS2::stdMsgs::SubscriberUInt64MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberUInt64MultiArray> Native;
};

struct FSubscriberUInt8 : public FROS2Subscriber
{
	FSubscriberUInt8(agxIO::ROS2::stdMsgs::SubscriberUInt8* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberUInt8> Native;
};

struct FSubscriberUInt8MultiArray : public FROS2Subscriber
{
	FSubscriberUInt8MultiArray(agxIO::ROS2::stdMsgs::SubscriberUInt8MultiArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberUInt8MultiArray> Native;
};

struct FSubscriberHeader : public FROS2Subscriber
{
	FSubscriberHeader(agxIO::ROS2::stdMsgs::SubscriberHeader* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::stdMsgs::SubscriberHeader> Native;
};

//
// GeometryMsgs
//

struct FPublisherVector3 : public FROS2Publisher
{
	FPublisherVector3(agxIO::ROS2::geometryMsgs::PublisherVector3* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherVector3> Native;
};

struct FPublisherQuaternion : public FROS2Publisher
{
	FPublisherQuaternion(agxIO::ROS2::geometryMsgs::PublisherQuaternion* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherQuaternion> Native;
};

struct FPublisherAccel : public FROS2Publisher
{
	FPublisherAccel(agxIO::ROS2::geometryMsgs::PublisherAccel* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherAccel> Native;
};

struct FPublisherAccelStamped : public FROS2Publisher
{
	FPublisherAccelStamped(agxIO::ROS2::geometryMsgs::PublisherAccelStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherAccelStamped> Native;
};

struct FPublisherAccelWithCovariance : public FROS2Publisher
{
	FPublisherAccelWithCovariance(agxIO::ROS2::geometryMsgs::PublisherAccelWithCovariance* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherAccelWithCovariance> Native;
};

struct FPublisherAccelWithCovarianceStamped : public FROS2Publisher
{
	FPublisherAccelWithCovarianceStamped(
		agxIO::ROS2::geometryMsgs::PublisherAccelWithCovarianceStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherAccelWithCovarianceStamped> Native;
};

struct FPublisherInertia : public FROS2Publisher
{
	FPublisherInertia(agxIO::ROS2::geometryMsgs::PublisherInertia* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherInertia> Native;
};

struct FPublisherInertiaStamped : public FROS2Publisher
{
	FPublisherInertiaStamped(agxIO::ROS2::geometryMsgs::PublisherInertiaStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherInertiaStamped> Native;
};

struct FPublisherPoint : public FROS2Publisher
{
	FPublisherPoint(agxIO::ROS2::geometryMsgs::PublisherPoint* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherPoint> Native;
};

struct FPublisherPoint32 : public FROS2Publisher
{
	FPublisherPoint32(agxIO::ROS2::geometryMsgs::PublisherPoint32* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherPoint32> Native;
};

struct FPublisherPointStamped : public FROS2Publisher
{
	FPublisherPointStamped(agxIO::ROS2::geometryMsgs::PublisherPointStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherPointStamped> Native;
};

struct FPublisherPolygon : public FROS2Publisher
{
	FPublisherPolygon(agxIO::ROS2::geometryMsgs::PublisherPolygon* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherPolygon> Native;
};

struct FPublisherPolygonStamped : public FROS2Publisher
{
	FPublisherPolygonStamped(agxIO::ROS2::geometryMsgs::PublisherPolygonStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherPolygonStamped> Native;
};

struct FPublisherPose : public FROS2Publisher
{
	FPublisherPose(agxIO::ROS2::geometryMsgs::PublisherPose* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherPose> Native;
};

struct FPublisherPose2D : public FROS2Publisher
{
	FPublisherPose2D(agxIO::ROS2::geometryMsgs::PublisherPose2D* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherPose2D> Native;
};

struct FPublisherPoseArray : public FROS2Publisher
{
	FPublisherPoseArray(agxIO::ROS2::geometryMsgs::PublisherPoseArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherPoseArray> Native;
};

struct FPublisherPoseStamped : public FROS2Publisher
{
	FPublisherPoseStamped(agxIO::ROS2::geometryMsgs::PublisherPoseStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherPoseStamped> Native;
};

struct FPublisherPoseWithCovariance : public FROS2Publisher
{
	FPublisherPoseWithCovariance(agxIO::ROS2::geometryMsgs::PublisherPoseWithCovariance* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherPoseWithCovariance> Native;
};

struct FPublisherPoseWithCovarianceStamped : public FROS2Publisher
{
	FPublisherPoseWithCovarianceStamped(
		agxIO::ROS2::geometryMsgs::PublisherPoseWithCovarianceStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherPoseWithCovarianceStamped> Native;
};

struct FPublisherQuaternionStamped : public FROS2Publisher
{
	FPublisherQuaternionStamped(agxIO::ROS2::geometryMsgs::PublisherQuaternionStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherQuaternionStamped> Native;
};

struct FPublisherTransform : public FROS2Publisher
{
	FPublisherTransform(agxIO::ROS2::geometryMsgs::PublisherTransform* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherTransform> Native;
};

struct FPublisherTransformStamped : public FROS2Publisher
{
	FPublisherTransformStamped(agxIO::ROS2::geometryMsgs::PublisherTransformStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherTransformStamped> Native;
};

struct FPublisherTwist : public FROS2Publisher
{
	FPublisherTwist(agxIO::ROS2::geometryMsgs::PublisherTwist* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherTwist> Native;
};

struct FPublisherTwistStamped : public FROS2Publisher
{
	FPublisherTwistStamped(agxIO::ROS2::geometryMsgs::PublisherTwistStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherTwistStamped> Native;
};

struct FPublisherTwistWithCovariance : public FROS2Publisher
{
	FPublisherTwistWithCovariance(agxIO::ROS2::geometryMsgs::PublisherTwistWithCovariance* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherTwistWithCovariance> Native;
};

struct FPublisherTwistWithCovarianceStamped : public FROS2Publisher
{
	FPublisherTwistWithCovarianceStamped(
		agxIO::ROS2::geometryMsgs::PublisherTwistWithCovarianceStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherTwistWithCovarianceStamped> Native;
};

struct FPublisherVector3Stamped : public FROS2Publisher
{
	FPublisherVector3Stamped(agxIO::ROS2::geometryMsgs::PublisherVector3Stamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherVector3Stamped> Native;
};

struct FPublisherWrench : public FROS2Publisher
{
	FPublisherWrench(agxIO::ROS2::geometryMsgs::PublisherWrench* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherWrench> Native;
};

struct FPublisherWrenchStamped : public FROS2Publisher
{
	FPublisherWrenchStamped(agxIO::ROS2::geometryMsgs::PublisherWrenchStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::PublisherWrenchStamped> Native;
};

struct FSubscriberVector3 : public FROS2Subscriber
{
	FSubscriberVector3(agxIO::ROS2::geometryMsgs::SubscriberVector3* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberVector3> Native;
};

struct FSubscriberQuaternion : public FROS2Subscriber
{
	FSubscriberQuaternion(agxIO::ROS2::geometryMsgs::SubscriberQuaternion* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberQuaternion> Native;
};

struct FSubscriberAccel : public FROS2Subscriber
{
	FSubscriberAccel(agxIO::ROS2::geometryMsgs::SubscriberAccel* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberAccel> Native;
};

struct FSubscriberAccelStamped : public FROS2Subscriber
{
	FSubscriberAccelStamped(agxIO::ROS2::geometryMsgs::SubscriberAccelStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberAccelStamped> Native;
};

struct FSubscriberAccelWithCovariance : public FROS2Subscriber
{
	FSubscriberAccelWithCovariance(agxIO::ROS2::geometryMsgs::SubscriberAccelWithCovariance* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberAccelWithCovariance> Native;
};

struct FSubscriberAccelWithCovarianceStamped : public FROS2Subscriber
{
	FSubscriberAccelWithCovarianceStamped(
		agxIO::ROS2::geometryMsgs::SubscriberAccelWithCovarianceStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberAccelWithCovarianceStamped> Native;
};

struct FSubscriberInertia : public FROS2Subscriber
{
	FSubscriberInertia(agxIO::ROS2::geometryMsgs::SubscriberInertia* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberInertia> Native;
};

struct FSubscriberInertiaStamped : public FROS2Subscriber
{
	FSubscriberInertiaStamped(agxIO::ROS2::geometryMsgs::SubscriberInertiaStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberInertiaStamped> Native;
};

struct FSubscriberPoint : public FROS2Subscriber
{
	FSubscriberPoint(agxIO::ROS2::geometryMsgs::SubscriberPoint* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberPoint> Native;
};

struct FSubscriberPoint32 : public FROS2Subscriber
{
	FSubscriberPoint32(agxIO::ROS2::geometryMsgs::SubscriberPoint32* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberPoint32> Native;
};

struct FSubscriberPointStamped : public FROS2Subscriber
{
	FSubscriberPointStamped(agxIO::ROS2::geometryMsgs::SubscriberPointStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberPointStamped> Native;
};

struct FSubscriberPolygon : public FROS2Subscriber
{
	FSubscriberPolygon(agxIO::ROS2::geometryMsgs::SubscriberPolygon* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberPolygon> Native;
};

struct FSubscriberPolygonStamped : public FROS2Subscriber
{
	FSubscriberPolygonStamped(agxIO::ROS2::geometryMsgs::SubscriberPolygonStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberPolygonStamped> Native;
};

struct FSubscriberPose : public FROS2Subscriber
{
	FSubscriberPose(agxIO::ROS2::geometryMsgs::SubscriberPose* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberPose> Native;
};

struct FSubscriberPose2D : public FROS2Subscriber
{
	FSubscriberPose2D(agxIO::ROS2::geometryMsgs::SubscriberPose2D* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberPose2D> Native;
};

struct FSubscriberPoseArray : public FROS2Subscriber
{
	FSubscriberPoseArray(agxIO::ROS2::geometryMsgs::SubscriberPoseArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberPoseArray> Native;
};

struct FSubscriberPoseStamped : public FROS2Subscriber
{
	FSubscriberPoseStamped(agxIO::ROS2::geometryMsgs::SubscriberPoseStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberPoseStamped> Native;
};

struct FSubscriberPoseWithCovariance : public FROS2Subscriber
{
	FSubscriberPoseWithCovariance(agxIO::ROS2::geometryMsgs::SubscriberPoseWithCovariance* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberPoseWithCovariance> Native;
};

struct FSubscriberPoseWithCovarianceStamped : public FROS2Subscriber
{
	FSubscriberPoseWithCovarianceStamped(
		agxIO::ROS2::geometryMsgs::SubscriberPoseWithCovarianceStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberPoseWithCovarianceStamped> Native;
};

struct FSubscriberQuaternionStamped : public FROS2Subscriber
{
	FSubscriberQuaternionStamped(agxIO::ROS2::geometryMsgs::SubscriberQuaternionStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberQuaternionStamped> Native;
};

struct FSubscriberTransform : public FROS2Subscriber
{
	FSubscriberTransform(agxIO::ROS2::geometryMsgs::SubscriberTransform* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberTransform> Native;
};

struct FSubscriberTransformStamped : public FROS2Subscriber
{
	FSubscriberTransformStamped(agxIO::ROS2::geometryMsgs::SubscriberTransformStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberTransformStamped> Native;
};

struct FSubscriberTwist : public FROS2Subscriber
{
	FSubscriberTwist(agxIO::ROS2::geometryMsgs::SubscriberTwist* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberTwist> Native;
};

struct FSubscriberTwistStamped : public FROS2Subscriber
{
	FSubscriberTwistStamped(agxIO::ROS2::geometryMsgs::SubscriberTwistStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberTwistStamped> Native;
};

struct FSubscriberTwistWithCovariance : public FROS2Subscriber
{
	FSubscriberTwistWithCovariance(agxIO::ROS2::geometryMsgs::SubscriberTwistWithCovariance* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberTwistWithCovariance> Native;
};

struct FSubscriberTwistWithCovarianceStamped : public FROS2Subscriber
{
	FSubscriberTwistWithCovarianceStamped(
		agxIO::ROS2::geometryMsgs::SubscriberTwistWithCovarianceStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberTwistWithCovarianceStamped> Native;
};

struct FSubscriberVector3Stamped : public FROS2Subscriber
{
	FSubscriberVector3Stamped(agxIO::ROS2::geometryMsgs::SubscriberVector3Stamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberVector3Stamped> Native;
};

struct FSubscriberWrench : public FROS2Subscriber
{
	FSubscriberWrench(agxIO::ROS2::geometryMsgs::SubscriberWrench* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberWrench> Native;
};

struct FSubscriberWrenchStamped : public FROS2Subscriber
{
	FSubscriberWrenchStamped(agxIO::ROS2::geometryMsgs::SubscriberWrenchStamped* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::geometryMsgs::SubscriberWrenchStamped> Native;
};

//
// SensorMsgs
//

struct FPublisherBatteryState : public FROS2Publisher
{
	FPublisherBatteryState(agxIO::ROS2::sensorMsgs::PublisherBatteryState* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::PublisherBatteryState> Native;
};

struct FPublisherChannelFloat32 : public FROS2Publisher
{
	FPublisherChannelFloat32(agxIO::ROS2::sensorMsgs::PublisherChannelFloat32* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::PublisherChannelFloat32> Native;
};

struct FPublisherCompressedImage : public FROS2Publisher
{
	FPublisherCompressedImage(agxIO::ROS2::sensorMsgs::PublisherCompressedImage* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::PublisherCompressedImage> Native;
};

struct FPublisherFluidPressure : public FROS2Publisher
{
	FPublisherFluidPressure(agxIO::ROS2::sensorMsgs::PublisherFluidPressure* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::PublisherFluidPressure> Native;
};

struct FPublisherIlluminance : public FROS2Publisher
{
	FPublisherIlluminance(agxIO::ROS2::sensorMsgs::PublisherIlluminance* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::PublisherIlluminance> Native;
};

struct FPublisherImage : public FROS2Publisher
{
	FPublisherImage(agxIO::ROS2::sensorMsgs::PublisherImage* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::PublisherImage> Native;
};

struct FPublisherImu : public FROS2Publisher
{
	FPublisherImu(agxIO::ROS2::sensorMsgs::PublisherImu* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::PublisherImu> Native;
};

struct FPublisherJointState : public FROS2Publisher
{
	FPublisherJointState(agxIO::ROS2::sensorMsgs::PublisherJointState* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::PublisherJointState> Native;
};

struct FPublisherJoy : public FROS2Publisher
{
	FPublisherJoy(agxIO::ROS2::sensorMsgs::PublisherJoy* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::PublisherJoy> Native;
};

struct FPublisherJoyFeedback : public FROS2Publisher
{
	FPublisherJoyFeedback(agxIO::ROS2::sensorMsgs::PublisherJoyFeedback* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::PublisherJoyFeedback> Native;
};

struct FPublisherJoyFeedbackArray : public FROS2Publisher
{
	FPublisherJoyFeedbackArray(agxIO::ROS2::sensorMsgs::PublisherJoyFeedbackArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::PublisherJoyFeedbackArray> Native;
};

struct FPublisherLaserEcho : public FROS2Publisher
{
	FPublisherLaserEcho(agxIO::ROS2::sensorMsgs::PublisherLaserEcho* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::PublisherLaserEcho> Native;
};

struct FPublisherLaserScan : public FROS2Publisher
{
	FPublisherLaserScan(agxIO::ROS2::sensorMsgs::PublisherLaserScan* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::PublisherLaserScan> Native;
};

struct FPublisherMagneticField : public FROS2Publisher
{
	FPublisherMagneticField(agxIO::ROS2::sensorMsgs::PublisherMagneticField* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::PublisherMagneticField> Native;
};

struct FPublisherMultiDOFJointState : public FROS2Publisher
{
	FPublisherMultiDOFJointState(agxIO::ROS2::sensorMsgs::PublisherMultiDOFJointState* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::PublisherMultiDOFJointState> Native;
};

struct FPublisherMultiEchoLaserScan : public FROS2Publisher
{
	FPublisherMultiEchoLaserScan(agxIO::ROS2::sensorMsgs::PublisherMultiEchoLaserScan* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::PublisherMultiEchoLaserScan> Native;
};

struct FPublisherNavSatStatus : public FROS2Publisher
{
	FPublisherNavSatStatus(agxIO::ROS2::sensorMsgs::PublisherNavSatStatus* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::PublisherNavSatStatus> Native;
};

struct FPublisherNavSatFix : public FROS2Publisher
{
	FPublisherNavSatFix(agxIO::ROS2::sensorMsgs::PublisherNavSatFix* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::PublisherNavSatFix> Native;
};

struct FPublisherPointCloud : public FROS2Publisher
{
	FPublisherPointCloud(agxIO::ROS2::sensorMsgs::PublisherPointCloud* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::PublisherPointCloud> Native;
};

struct FPublisherPointField : public FROS2Publisher
{
	FPublisherPointField(agxIO::ROS2::sensorMsgs::PublisherPointField* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::PublisherPointField> Native;
};

struct FPublisherPointCloud2 : public FROS2Publisher
{
	FPublisherPointCloud2(agxIO::ROS2::sensorMsgs::PublisherPointCloud2* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::PublisherPointCloud2> Native;
};

struct FPublisherRange : public FROS2Publisher
{
	FPublisherRange(agxIO::ROS2::sensorMsgs::PublisherRange* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::PublisherRange> Native;
};

struct FPublisherRegionOfInterest : public FROS2Publisher
{
	FPublisherRegionOfInterest(agxIO::ROS2::sensorMsgs::PublisherRegionOfInterest* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::PublisherRegionOfInterest> Native;
};

struct FPublisherCameraInfo : public FROS2Publisher
{
	FPublisherCameraInfo(agxIO::ROS2::sensorMsgs::PublisherCameraInfo* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::PublisherCameraInfo> Native;
};

struct FPublisherRelativeHumidity : public FROS2Publisher
{
	FPublisherRelativeHumidity(agxIO::ROS2::sensorMsgs::PublisherRelativeHumidity* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::PublisherRelativeHumidity> Native;
};

struct FPublisherTemperature : public FROS2Publisher
{
	FPublisherTemperature(agxIO::ROS2::sensorMsgs::PublisherTemperature* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::PublisherTemperature> Native;
};

struct FPublisherTimeReference : public FROS2Publisher
{
	FPublisherTimeReference(agxIO::ROS2::sensorMsgs::PublisherTimeReference* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::PublisherTimeReference> Native;
};

struct FSubscriberBatteryState : public FROS2Subscriber
{
	FSubscriberBatteryState(agxIO::ROS2::sensorMsgs::SubscriberBatteryState* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::SubscriberBatteryState> Native;
};

struct FSubscriberChannelFloat32 : public FROS2Subscriber
{
	FSubscriberChannelFloat32(agxIO::ROS2::sensorMsgs::SubscriberChannelFloat32* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::SubscriberChannelFloat32> Native;
};

struct FSubscriberCompressedImage : public FROS2Subscriber
{
	FSubscriberCompressedImage(agxIO::ROS2::sensorMsgs::SubscriberCompressedImage* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::SubscriberCompressedImage> Native;
};

struct FSubscriberFluidPressure : public FROS2Subscriber
{
	FSubscriberFluidPressure(agxIO::ROS2::sensorMsgs::SubscriberFluidPressure* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::SubscriberFluidPressure> Native;
};

struct FSubscriberIlluminance : public FROS2Subscriber
{
	FSubscriberIlluminance(agxIO::ROS2::sensorMsgs::SubscriberIlluminance* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::SubscriberIlluminance> Native;
};

struct FSubscriberImage : public FROS2Subscriber
{
	FSubscriberImage(agxIO::ROS2::sensorMsgs::SubscriberImage* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::SubscriberImage> Native;
};

struct FSubscriberImu : public FROS2Subscriber
{
	FSubscriberImu(agxIO::ROS2::sensorMsgs::SubscriberImu* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::SubscriberImu> Native;
};

struct FSubscriberJointState : public FROS2Subscriber
{
	FSubscriberJointState(agxIO::ROS2::sensorMsgs::SubscriberJointState* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::SubscriberJointState> Native;
};

struct FSubscriberJoy : public FROS2Subscriber
{
	FSubscriberJoy(agxIO::ROS2::sensorMsgs::SubscriberJoy* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::SubscriberJoy> Native;
};

struct FSubscriberJoyFeedback : public FROS2Subscriber
{
	FSubscriberJoyFeedback(agxIO::ROS2::sensorMsgs::SubscriberJoyFeedback* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::SubscriberJoyFeedback> Native;
};

struct FSubscriberJoyFeedbackArray : public FROS2Subscriber
{
	FSubscriberJoyFeedbackArray(agxIO::ROS2::sensorMsgs::SubscriberJoyFeedbackArray* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::SubscriberJoyFeedbackArray> Native;
};

struct FSubscriberLaserEcho : public FROS2Subscriber
{
	FSubscriberLaserEcho(agxIO::ROS2::sensorMsgs::SubscriberLaserEcho* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::SubscriberLaserEcho> Native;
};

struct FSubscriberLaserScan : public FROS2Subscriber
{
	FSubscriberLaserScan(agxIO::ROS2::sensorMsgs::SubscriberLaserScan* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::SubscriberLaserScan> Native;
};

struct FSubscriberMagneticField : public FROS2Subscriber
{
	FSubscriberMagneticField(agxIO::ROS2::sensorMsgs::SubscriberMagneticField* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::SubscriberMagneticField> Native;
};

struct FSubscriberMultiDOFJointState : public FROS2Subscriber
{
	FSubscriberMultiDOFJointState(agxIO::ROS2::sensorMsgs::SubscriberMultiDOFJointState* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::SubscriberMultiDOFJointState> Native;
};

struct FSubscriberMultiEchoLaserScan : public FROS2Subscriber
{
	FSubscriberMultiEchoLaserScan(agxIO::ROS2::sensorMsgs::SubscriberMultiEchoLaserScan* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::SubscriberMultiEchoLaserScan> Native;
};

struct FSubscriberNavSatStatus : public FROS2Subscriber
{
	FSubscriberNavSatStatus(agxIO::ROS2::sensorMsgs::SubscriberNavSatStatus* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::SubscriberNavSatStatus> Native;
};

struct FSubscriberNavSatFix : public FROS2Subscriber
{
	FSubscriberNavSatFix(agxIO::ROS2::sensorMsgs::SubscriberNavSatFix* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::SubscriberNavSatFix> Native;
};

struct FSubscriberPointCloud : public FROS2Subscriber
{
	FSubscriberPointCloud(agxIO::ROS2::sensorMsgs::SubscriberPointCloud* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::SubscriberPointCloud> Native;
};

struct FSubscriberPointField : public FROS2Subscriber
{
	FSubscriberPointField(agxIO::ROS2::sensorMsgs::SubscriberPointField* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::SubscriberPointField> Native;
};

struct FSubscriberPointCloud2 : public FROS2Subscriber
{
	FSubscriberPointCloud2(agxIO::ROS2::sensorMsgs::SubscriberPointCloud2* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::SubscriberPointCloud2> Native;
};

struct FSubscriberRange : public FROS2Subscriber
{
	FSubscriberRange(agxIO::ROS2::sensorMsgs::SubscriberRange* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::SubscriberRange> Native;
};

struct FSubscriberRegionOfInterest : public FROS2Subscriber
{
	FSubscriberRegionOfInterest(agxIO::ROS2::sensorMsgs::SubscriberRegionOfInterest* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::SubscriberRegionOfInterest> Native;
};

struct FSubscriberCameraInfo : public FROS2Subscriber
{
	FSubscriberCameraInfo(agxIO::ROS2::sensorMsgs::SubscriberCameraInfo* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::SubscriberCameraInfo> Native;
};

struct FSubscriberRelativeHumidity : public FROS2Subscriber
{
	FSubscriberRelativeHumidity(agxIO::ROS2::sensorMsgs::SubscriberRelativeHumidity* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::SubscriberRelativeHumidity> Native;
};

struct FSubscriberTemperature : public FROS2Subscriber
{
	FSubscriberTemperature(agxIO::ROS2::sensorMsgs::SubscriberTemperature* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::SubscriberTemperature> Native;
};

struct FSubscriberTimeReference : public FROS2Subscriber
{
	FSubscriberTimeReference(agxIO::ROS2::sensorMsgs::SubscriberTimeReference* Pub)
		: Native(Pub)
	{
	}

	std::unique_ptr<agxIO::ROS2::sensorMsgs::SubscriberTimeReference> Native;
};
