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
// AgxMsgs
//

FAGX_AgxMsgsAny Convert(const agxIO::ROS2::agxMsgs::Any& InMsg)
{
	FAGX_AgxMsgsAny Msg;

	Msg.Data.Reserve(InMsg.data.size());
	for (const auto& Value : InMsg.data)
	{
		Msg.Data.Add(Value);
	}

	return Msg;
}

FAGX_AgxMsgsAnySequence Convert(const agxIO::ROS2::agxMsgs::AnySequence& InMsg)
{
	FAGX_AgxMsgsAnySequence Msg;
	Msg.Data.SetNum(InMsg.data.size());

	for (int32 i = 0; i < InMsg.data.size(); ++i)
	{
		Msg.Data[i] = Convert(InMsg.data[i]);
	}

	return Msg;
}

//
// BuiltinInterfaces
//

FAGX_BuiltinInterfacesTime Convert(const agxIO::ROS2::builtinInterfaces::Time& InMsg)
{
	FAGX_BuiltinInterfacesTime Msg;
	Msg.Sec = InMsg.sec;
	Msg.Nanosec = static_cast<int32>(InMsg.nanosec);
	return Msg;
}

FAGX_BuiltinInterfacesDuration Convert(const agxIO::ROS2::builtinInterfaces::Duration& InMsg)
{
	FAGX_BuiltinInterfacesDuration Msg;
	Msg.Sec = InMsg.sec;
	Msg.Nanosec = static_cast<int32>(InMsg.nanosec);
	return Msg;
}

//
// RosgraphMsgs
//

FAGX_RosgraphMsgsClock Convert(const agxIO::ROS2::rosgraphMsgs::Clock& InMsg)
{
	FAGX_RosgraphMsgsClock Msg;
	Msg.Clock = Convert(InMsg.clock);
	return Msg;
}

//
// StdMsgs
//

FAGX_StdMsgsMultiArrayDimension Convert(const agxIO::ROS2::stdMsgs::MultiArrayDimension& InMsg)
{
	FAGX_StdMsgsMultiArrayDimension Msg;
	Msg.Label = FString(InMsg.label.c_str());
	Msg.Size = static_cast<int32>(InMsg.size);
	Msg.Stride = static_cast<int32>(InMsg.stride);
	return Msg;
}

FAGX_StdMsgsMultiArrayLayout Convert(const agxIO::ROS2::stdMsgs::MultiArrayLayout& InMsg)
{
	FAGX_StdMsgsMultiArrayLayout Msg;
	Msg.DataOffset = static_cast<int32>(InMsg.data_offset);

	for (const auto& dimension : InMsg.dim)
	{
		Msg.Dim.Add(Convert(dimension));
	}

	return Msg;
}

FAGX_StdMsgsBool Convert(const agxIO::ROS2::stdMsgs::Bool& InMsg)
{
	FAGX_StdMsgsBool Msg;
	Msg.Data = InMsg.data;
	return Msg;
}

FAGX_StdMsgsByte Convert(const agxIO::ROS2::stdMsgs::Byte& InMsg)
{
	FAGX_StdMsgsByte Msg;
	Msg.Data = InMsg.data;
	return Msg;
}

FAGX_StdMsgsByteMultiArray Convert(const agxIO::ROS2::stdMsgs::ByteMultiArray& InMsg)
{
	FAGX_StdMsgsByteMultiArray Msg;
	Msg.Layout = Convert(InMsg.layout);

	Msg.Data.Reserve(InMsg.data.size());
	for (const auto& Value : InMsg.data)
	{
		Msg.Data.Add(Value);
	}

	return Msg;
}

FAGX_StdMsgsChar Convert(const agxIO::ROS2::stdMsgs::Char& InMsg)
{
	FAGX_StdMsgsChar Msg;
	Msg.Data = InMsg.data;
	return Msg;
}

FAGX_StdMsgsColorRGBA Convert(const agxIO::ROS2::stdMsgs::ColorRGBA& InMsg)
{
	FAGX_StdMsgsColorRGBA Msg;
	Msg.R = InMsg.r;
	Msg.G = InMsg.g;
	Msg.B = InMsg.b;
	Msg.A = InMsg.a;
	return Msg;
}

FAGX_StdMsgsEmpty Convert(const agxIO::ROS2::stdMsgs::Empty& InMsg)
{
	FAGX_StdMsgsEmpty Msg;
	Msg.StructureNeedsAtLeastOneMember = InMsg.structure_needs_at_least_one_member;
	return Msg;
}

FAGX_StdMsgsFloat32 Convert(const agxIO::ROS2::stdMsgs::Float32& InMsg)
{
	FAGX_StdMsgsFloat32 Msg;
	Msg.Data = InMsg.data;
	return Msg;
}

FAGX_StdMsgsFloat32MultiArray Convert(const agxIO::ROS2::stdMsgs::Float32MultiArray& InMsg)
{
	FAGX_StdMsgsFloat32MultiArray Msg;
	Msg.Layout = Convert(InMsg.layout);

	Msg.Data.Reserve(InMsg.data.size());
	for (const auto& Value : InMsg.data)
	{
		Msg.Data.Add(Value);
	}

	return Msg;
}

FAGX_StdMsgsFloat64 Convert(const agxIO::ROS2::stdMsgs::Float64& InMsg)
{
	FAGX_StdMsgsFloat64 Msg;
	Msg.Data = InMsg.data;
	return Msg;
}

FAGX_StdMsgsFloat64MultiArray Convert(const agxIO::ROS2::stdMsgs::Float64MultiArray& InMsg)
{
	FAGX_StdMsgsFloat64MultiArray Msg;
	Msg.Layout = Convert(InMsg.layout);

	Msg.Data.Reserve(InMsg.data.size());
	for (const auto& Value : InMsg.data)
	{
		Msg.Data.Add(Value);
	}

	return Msg;
}

FAGX_StdMsgsInt16 Convert(const agxIO::ROS2::stdMsgs::Int16& InMsg)
{
	FAGX_StdMsgsInt16 Msg;
	Msg.Data = static_cast<int32>(InMsg.data);
	return Msg;
}

FAGX_StdMsgsInt16MultiArray Convert(const agxIO::ROS2::stdMsgs::Int16MultiArray& InMsg)
{
	FAGX_StdMsgsInt16MultiArray Msg;
	Msg.Layout = Convert(InMsg.layout);
	Msg.Data.SetNum(InMsg.data.size());

	for (int32 i = 0; i < InMsg.data.size(); ++i)
	{
		Msg.Data[i] = static_cast<int32>(InMsg.data[i]);
	}

	return Msg;
}

FAGX_StdMsgsInt32 Convert(const agxIO::ROS2::stdMsgs::Int32& InMsg)
{
	FAGX_StdMsgsInt32 Msg;
	Msg.Data = InMsg.data;
	return Msg;
}

FAGX_StdMsgsInt32MultiArray Convert(const agxIO::ROS2::stdMsgs::Int32MultiArray& InMsg)
{
	FAGX_StdMsgsInt32MultiArray Msg;
	Msg.Layout = Convert(InMsg.layout);

	Msg.Data.Reserve(InMsg.data.size());
	for (const auto& Value : InMsg.data)
	{
		Msg.Data.Add(Value);
	}

	return Msg;
}

FAGX_StdMsgsInt64 Convert(const agxIO::ROS2::stdMsgs::Int64& InMsg)
{
	FAGX_StdMsgsInt64 Msg;
	Msg.Data = InMsg.data;
	return Msg;
}

FAGX_StdMsgsInt64MultiArray Convert(const agxIO::ROS2::stdMsgs::Int64MultiArray& InMsg)
{
	FAGX_StdMsgsInt64MultiArray Msg;
	Msg.Layout = Convert(InMsg.layout);

	Msg.Data.Reserve(InMsg.data.size());
	for (const auto& Value : InMsg.data)
	{
		Msg.Data.Add(Value);
	}

	return Msg;
}

FAGX_StdMsgsInt8 Convert(const agxIO::ROS2::stdMsgs::Int8& InMsg)
{
	FAGX_StdMsgsInt8 Msg;
	Msg.Data = static_cast<uint8>(InMsg.data);
	return Msg;
}

FAGX_StdMsgsInt8MultiArray Convert(const agxIO::ROS2::stdMsgs::Int8MultiArray& InMsg)
{
	FAGX_StdMsgsInt8MultiArray Msg;
	Msg.Layout = Convert(InMsg.layout);
	Msg.Data.SetNum(InMsg.data.size());

	for (int32 i = 0; i < InMsg.data.size(); ++i)
	{
		Msg.Data[i] = static_cast<uint8>(InMsg.data[i]);
	}

	return Msg;
}

FAGX_StdMsgsString Convert(const agxIO::ROS2::stdMsgs::String& InMsg)
{
	FAGX_StdMsgsString Msg;
	Msg.Data = FString(InMsg.data.c_str());
	return Msg;
}

FAGX_StdMsgsUInt16 Convert(const agxIO::ROS2::stdMsgs::UInt16& InMsg)
{
	FAGX_StdMsgsUInt16 Msg;
	Msg.Data = static_cast<int32>(InMsg.data);
	return Msg;
}

FAGX_StdMsgsUInt16MultiArray Convert(const agxIO::ROS2::stdMsgs::UInt16MultiArray& InMsg)
{
	FAGX_StdMsgsUInt16MultiArray Msg;
	Msg.Layout = Convert(InMsg.layout);
	Msg.Data.SetNum(InMsg.data.size());

	for (int32 i = 0; i < InMsg.data.size(); ++i)
	{
		Msg.Data[i] = static_cast<int32>(InMsg.data[i]);
	}

	return Msg;
}

FAGX_StdMsgsUInt32 Convert(const agxIO::ROS2::stdMsgs::UInt32& InMsg)
{
	FAGX_StdMsgsUInt32 Msg;
	Msg.Data = static_cast<int32>(InMsg.data);
	return Msg;
}

FAGX_StdMsgsUInt32MultiArray Convert(const agxIO::ROS2::stdMsgs::UInt32MultiArray& InMsg)
{
	FAGX_StdMsgsUInt32MultiArray Msg;
	Msg.Layout = Convert(InMsg.layout);

	Msg.Data.SetNum(InMsg.data.size());
	for (int32 i = 0; i < InMsg.data.size(); ++i)
	{
		Msg.Data[i] = static_cast<int32>(InMsg.data[i]);
	}

	return Msg;
}

FAGX_StdMsgsUInt64 Convert(const agxIO::ROS2::stdMsgs::UInt64& InMsg)
{
	FAGX_StdMsgsUInt64 Msg;
	Msg.Data = static_cast<int32>(InMsg.data);
	return Msg;
}

FAGX_StdMsgsUInt64MultiArray Convert(const agxIO::ROS2::stdMsgs::UInt64MultiArray& InMsg)
{
	FAGX_StdMsgsUInt64MultiArray Msg;
	Msg.Layout = Convert(InMsg.layout);
	Msg.Data.SetNum(InMsg.data.size());

	for (int32 i = 0; i < InMsg.data.size(); ++i)
	{
		Msg.Data[i] = static_cast<int32>(InMsg.data[i]);
	}

	return Msg;
}

FAGX_StdMsgsUInt8 Convert(const agxIO::ROS2::stdMsgs::UInt8& InMsg)
{
	FAGX_StdMsgsUInt8 Msg;
	Msg.Data = InMsg.data;
	return Msg;
}

FAGX_StdMsgsUInt8MultiArray Convert(const agxIO::ROS2::stdMsgs::UInt8MultiArray& InMsg)
{
	FAGX_StdMsgsUInt8MultiArray Msg;
	Msg.Layout = Convert(InMsg.layout);

	Msg.Data.Reserve(InMsg.data.size());
	for (const auto& Value : InMsg.data)
	{
		Msg.Data.Add(Value);
	}

	return Msg;
}

FAGX_StdMsgsHeader Convert(const agxIO::ROS2::stdMsgs::Header& InMsg)
{
	FAGX_StdMsgsHeader Msg;
	Msg.Stamp = Convert(InMsg.stamp);
	Msg.FrameId = FString(InMsg.frame_id.c_str());
	return Msg;
}

//
// GeometryMsgs
//

FAGX_GeometryMsgsVector3 Convert(const agxIO::ROS2::geometryMsgs::Vector3& InMsg)
{
	FAGX_GeometryMsgsVector3 Msg;
	Msg.X = InMsg.x;
	Msg.Y = InMsg.y;
	Msg.Z = InMsg.z;
	return Msg;
}

FAGX_GeometryMsgsQuaternion Convert(const agxIO::ROS2::geometryMsgs::Quaternion& InMsg)
{
	FAGX_GeometryMsgsQuaternion Msg;
	Msg.X = InMsg.x;
	Msg.Y = InMsg.y;
	Msg.Z = InMsg.z;
	Msg.W = InMsg.w;
	return Msg;
}

FAGX_GeometryMsgsAccel Convert(const agxIO::ROS2::geometryMsgs::Accel& InMsg)
{
	FAGX_GeometryMsgsAccel Msg;
	Msg.Linear = Convert(InMsg.linear);
	Msg.Angular = Convert(InMsg.angular);
	return Msg;
}

FAGX_GeometryMsgsAccelStamped Convert(const agxIO::ROS2::geometryMsgs::AccelStamped& InMsg)
{
	FAGX_GeometryMsgsAccelStamped Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Accel = Convert(InMsg.accel);
	return Msg;
}

FAGX_GeometryMsgsAccelWithCovariance Convert(
	const agxIO::ROS2::geometryMsgs::AccelWithCovariance& InMsg)
{
	FAGX_GeometryMsgsAccelWithCovariance Msg;
	Msg.Accel = Convert(InMsg.accel);

	// Note: Covariance is a dynamic array on the Unreal side.
	Msg.Covariance.SetNum(36);
	for (int32 i = 0; i < 36; ++i)
	{
		Msg.Covariance[i] = static_cast<double>(InMsg.covariance[i]);
	}
	return Msg;
}

FAGX_GeometryMsgsAccelWithCovarianceStamped Convert(
	const agxIO::ROS2::geometryMsgs::AccelWithCovarianceStamped& InMsg)
{
	FAGX_GeometryMsgsAccelWithCovarianceStamped Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Accel = Convert(InMsg.accel);
	return Msg;
}

FAGX_GeometryMsgsInertia Convert(const agxIO::ROS2::geometryMsgs::Inertia& InMsg)
{
	FAGX_GeometryMsgsInertia Msg;
	Msg.M = InMsg.m;
	Msg.COM = Convert(InMsg.com);
	Msg.Ixx = InMsg.ixx;
	Msg.Ixy = InMsg.ixy;
	Msg.Ixz = InMsg.ixz;
	Msg.Iyy = InMsg.iyy;
	Msg.Iyz = InMsg.iyz;
	Msg.Izz = InMsg.izz;
	return Msg;
}

FAGX_GeometryMsgsInertiaStamped Convert(const agxIO::ROS2::geometryMsgs::InertiaStamped& InMsg)
{
	FAGX_GeometryMsgsInertiaStamped Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Inertia = Convert(InMsg.inertia);
	return Msg;
}

FAGX_GeometryMsgsPoint Convert(const agxIO::ROS2::geometryMsgs::Point& InMsg)
{
	FAGX_GeometryMsgsPoint Msg;
	Msg.X = InMsg.x;
	Msg.Y = InMsg.y;
	Msg.Z = InMsg.z;
	return Msg;
}

FAGX_GeometryMsgsPoint32 Convert(const agxIO::ROS2::geometryMsgs::Point32& InMsg)
{
	FAGX_GeometryMsgsPoint32 Msg;
	Msg.X = InMsg.x;
	Msg.Y = InMsg.y;
	Msg.Z = InMsg.z;
	return Msg;
}

FAGX_GeometryMsgsPointStamped Convert(const agxIO::ROS2::geometryMsgs::PointStamped& InMsg)
{
	FAGX_GeometryMsgsPointStamped Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Point = Convert(InMsg.point);
	return Msg;
}

FAGX_GeometryMsgsPolygon Convert(const agxIO::ROS2::geometryMsgs::Polygon& InMsg)
{
	FAGX_GeometryMsgsPolygon Msg;
	for (const auto& point : InMsg.points)
	{
		Msg.Points.Add(Convert(point));
	}
	return Msg;
}

FAGX_GeometryMsgsPolygonStamped Convert(const agxIO::ROS2::geometryMsgs::PolygonStamped& InMsg)
{
	FAGX_GeometryMsgsPolygonStamped Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Polygon = Convert(InMsg.polygon);
	return Msg;
}

FAGX_GeometryMsgsPose Convert(const agxIO::ROS2::geometryMsgs::Pose& InMsg)
{
	FAGX_GeometryMsgsPose Msg;
	Msg.Position = Convert(InMsg.position);
	Msg.Orientation = Convert(InMsg.orientation);
	return Msg;
}

FAGX_GeometryMsgsPose2D Convert(const agxIO::ROS2::geometryMsgs::Pose2D& InMsg)
{
	FAGX_GeometryMsgsPose2D Msg;
	Msg.X = InMsg.x;
	Msg.Y = InMsg.y;
	Msg.Theta = InMsg.theta;
	return Msg;
}

FAGX_GeometryMsgsPoseArray Convert(const agxIO::ROS2::geometryMsgs::PoseArray& InMsg)
{
	FAGX_GeometryMsgsPoseArray Msg;
	Msg.Header = Convert(InMsg.header);
	for (const auto& pose : InMsg.poses)
	{
		Msg.Poses.Add(Convert(pose));
	}
	return Msg;
}

FAGX_GeometryMsgsPoseStamped Convert(const agxIO::ROS2::geometryMsgs::PoseStamped& InMsg)
{
	FAGX_GeometryMsgsPoseStamped Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Pose = Convert(InMsg.pose);
	return Msg;
}

FAGX_GeometryMsgsPoseWithCovariance Convert(
	const agxIO::ROS2::geometryMsgs::PoseWithCovariance& InMsg)
{
	FAGX_GeometryMsgsPoseWithCovariance Msg;
	Msg.Pose = Convert(InMsg.pose);

	// Note: Covariance is a dynamic array on the Unreal side.
	Msg.Covariance.SetNum(36);
	for (int32 i = 0; i < 36; ++i)
	{
		Msg.Covariance[i] = static_cast<double>(InMsg.covariance[i]);
	}
	return Msg;
}

FAGX_GeometryMsgsPoseWithCovarianceStamped Convert(
	const agxIO::ROS2::geometryMsgs::PoseWithCovarianceStamped& InMsg)
{
	FAGX_GeometryMsgsPoseWithCovarianceStamped Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Pose = Convert(InMsg.pose);
	return Msg;
}

FAGX_GeometryMsgsQuaternionStamped Convert(
	const agxIO::ROS2::geometryMsgs::QuaternionStamped& InMsg)
{
	FAGX_GeometryMsgsQuaternionStamped Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Quaternion = Convert(InMsg.quaternion);
	return Msg;
}

FAGX_GeometryMsgsTransform Convert(const agxIO::ROS2::geometryMsgs::Transform& InMsg)
{
	FAGX_GeometryMsgsTransform Msg;
	Msg.Translation = Convert(InMsg.translation);
	Msg.Rotation = Convert(InMsg.rotation);
	return Msg;
}

FAGX_GeometryMsgsTransformStamped Convert(const agxIO::ROS2::geometryMsgs::TransformStamped& InMsg)
{
	FAGX_GeometryMsgsTransformStamped Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.ChildFrameId = FString(InMsg.child_frame_id.c_str());
	Msg.Transform = Convert(InMsg.transform);
	return Msg;
}

FAGX_GeometryMsgsTwist Convert(const agxIO::ROS2::geometryMsgs::Twist& InMsg)
{
	FAGX_GeometryMsgsTwist Msg;
	Msg.Linear = Convert(InMsg.linear);
	Msg.Angular = Convert(InMsg.angular);
	return Msg;
}

FAGX_GeometryMsgsTwistStamped Convert(const agxIO::ROS2::geometryMsgs::TwistStamped& InMsg)
{
	FAGX_GeometryMsgsTwistStamped Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Twist = Convert(InMsg.twist);
	return Msg;
}

FAGX_GeometryMsgsTwistWithCovariance Convert(
	const agxIO::ROS2::geometryMsgs::TwistWithCovariance& InMsg)
{
	FAGX_GeometryMsgsTwistWithCovariance Msg;
	Msg.Twist = Convert(InMsg.twist);

	// Note: Covariance is a dynamic array on the Unreal side.
	Msg.Covariance.SetNum(36);
	for (int32 i = 0; i < 36; ++i)
	{
		Msg.Covariance[i] = static_cast<double>(InMsg.covariance[i]);
	}
	return Msg;
}

FAGX_GeometryMsgsTwistWithCovarianceStamped Convert(
	const agxIO::ROS2::geometryMsgs::TwistWithCovarianceStamped& InMsg)
{
	FAGX_GeometryMsgsTwistWithCovarianceStamped Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Twist = Convert(InMsg.twist);
	return Msg;
}

FAGX_GeometryMsgsVector3Stamped Convert(const agxIO::ROS2::geometryMsgs::Vector3Stamped& InMsg)
{
	FAGX_GeometryMsgsVector3Stamped Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Vector = Convert(InMsg.vector);
	return Msg;
}

FAGX_GeometryMsgsWrench Convert(const agxIO::ROS2::geometryMsgs::Wrench& InMsg)
{
	FAGX_GeometryMsgsWrench Msg;
	Msg.Force = Convert(InMsg.force);
	Msg.Torque = Convert(InMsg.torque);
	return Msg;
}

FAGX_GeometryMsgsWrenchStamped Convert(const agxIO::ROS2::geometryMsgs::WrenchStamped& InMsg)
{
	FAGX_GeometryMsgsWrenchStamped Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Wrench = Convert(InMsg.wrench);
	return Msg;
}

//
// SensorMsgs
//

FAGX_SensorMsgsBatteryState Convert(const agxIO::ROS2::sensorMsgs::BatteryState& InMsg)
{
	FAGX_SensorMsgsBatteryState Msg;

	Msg.Header = Convert(InMsg.header);

	Msg.Voltage = InMsg.voltage;
	Msg.Temperature = InMsg.temperature;
	Msg.Current = InMsg.current;
	Msg.Charge = InMsg.charge;
	Msg.Capacity = InMsg.capacity;
	Msg.DesignCapacity = InMsg.design_capacity;
	Msg.Percentage = InMsg.percentage;
	Msg.PowerSupplyStatus = InMsg.power_supply_status;
	Msg.PowerSupplyHealth = InMsg.power_supply_health;
	Msg.PowerSupplyTechnology = InMsg.power_supply_technology;
	Msg.Present = InMsg.present;

	Msg.CellVoltage.Reserve(InMsg.cell_voltage.size());
	for (const auto& Value : InMsg.cell_voltage)
	{
		Msg.CellVoltage.Add(Value);
	}

	Msg.CellTemperature.Reserve(InMsg.cell_temperature.size());
	for (const auto& Value : InMsg.cell_temperature)
	{
		Msg.CellTemperature.Add(Value);
	}

	Msg.Location = FString(InMsg.location.c_str());
	Msg.SerialNumber = FString(InMsg.serial_number.c_str());

	return Msg;
}

FAGX_SensorMsgsChannelFloat32 Convert(const agxIO::ROS2::sensorMsgs::ChannelFloat32& InMsg)
{
	FAGX_SensorMsgsChannelFloat32 Msg;
	Msg.Name = FString(InMsg.name.c_str());

	Msg.Values.SetNum(InMsg.values.size());
	for (int32 i = 0; i < InMsg.values.size(); ++i)
		Msg.Values[i] = InMsg.values[i];

	return Msg;
}

FAGX_SensorMsgsCompressedImage Convert(const agxIO::ROS2::sensorMsgs::CompressedImage& InMsg)
{
	FAGX_SensorMsgsCompressedImage Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Format = FString(InMsg.format.c_str());

	Msg.Data.SetNum(InMsg.data.size());
	for (int32 i = 0; i < InMsg.data.size(); ++i)
		Msg.Data[i] = InMsg.data[i];

	return Msg;
}

FAGX_SensorMsgsFluidPressure Convert(const agxIO::ROS2::sensorMsgs::FluidPressure& InMsg)
{
	FAGX_SensorMsgsFluidPressure Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.FluidPressure = InMsg.fluid_pressure;
	Msg.Variance = InMsg.variance;
	return Msg;
}

FAGX_SensorMsgsIlluminance Convert(const agxIO::ROS2::sensorMsgs::Illuminance& InMsg)
{
	FAGX_SensorMsgsIlluminance Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Illuminance = InMsg.illuminance;
	Msg.Variance = InMsg.variance;
	return Msg;
}

FAGX_SensorMsgsImage Convert(const agxIO::ROS2::sensorMsgs::Image& InMsg)
{
	FAGX_SensorMsgsImage Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Height = static_cast<int32>(InMsg.height);
	Msg.Width = static_cast<int32>(InMsg.width);
	Msg.Encoding = FString(InMsg.encoding.c_str());
	Msg.IsBigendian = static_cast<uint8>(InMsg.is_bigendian);
	Msg.Step = static_cast<int32>(InMsg.step);

	Msg.Data.SetNum(InMsg.data.size());
	for (int32 i = 0; i < InMsg.data.size(); ++i)
		Msg.Data[i] = InMsg.data[i];

	return Msg;
}

FAGX_SensorMsgsImu Convert(const agxIO::ROS2::sensorMsgs::Imu& InMsg)
{
	FAGX_SensorMsgsImu Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Orientation = Convert(InMsg.orientation);

	// Note: This is a dynamic array on the Unreal side, static array on agxROS2 side.
	Msg.OrientationCovariance.SetNum(9);
	for (int32 i = 0; i < 9; ++i)
		Msg.OrientationCovariance[i] = InMsg.orientation_covariance[i];

	Msg.AngularVelocity = Convert(InMsg.angular_velocity);

	// Note: This is a dynamic array on the Unreal side, static array on agxROS2 side.
	Msg.AngularVelocityCovariance.SetNum(9);
	for (int32 i = 0; i < 9; ++i)
		Msg.AngularVelocityCovariance[i] = InMsg.angular_velocity_covariance[i];

	Msg.LinearAcceleration = Convert(InMsg.linear_acceleration);

	// Note: This is a dynamic array on the Unreal side, static array on agxROS2 side.
	Msg.LinearAccelerationCovariance.SetNum(9);
	for (int32 i = 0; i < 9; ++i)
		Msg.LinearAccelerationCovariance[i] = InMsg.linear_acceleration_covariance[i];

	return Msg;
}

FAGX_SensorMsgsJointState Convert(const agxIO::ROS2::sensorMsgs::JointState& InMsg)
{
	FAGX_SensorMsgsJointState Msg;
	Msg.Header = Convert(InMsg.header);

	Msg.Name.SetNum(InMsg.name.size());
	for (int32 i = 0; i < InMsg.name.size(); ++i)
		Msg.Name[i] = FString(InMsg.name[i].c_str());

	Msg.Position.SetNum(InMsg.position.size());
	for (int32 i = 0; i < InMsg.position.size(); ++i)
		Msg.Position[i] = InMsg.position[i];

	Msg.Velocity.SetNum(InMsg.velocity.size());
	for (int32 i = 0; i < InMsg.velocity.size(); ++i)
		Msg.Velocity[i] = InMsg.velocity[i];

	Msg.Effort.SetNum(InMsg.effort.size());
	for (int32 i = 0; i < InMsg.effort.size(); ++i)
		Msg.Effort[i] = InMsg.effort[i];

	return Msg;
}

FAGX_SensorMsgsJoy Convert(const agxIO::ROS2::sensorMsgs::Joy& InMsg)
{
	FAGX_SensorMsgsJoy Msg;
	Msg.Header = Convert(InMsg.header);

	Msg.Axes.SetNum(InMsg.axes.size());
	for (int32 i = 0; i < InMsg.axes.size(); ++i)
		Msg.Axes[i] = InMsg.axes[i];

	Msg.Buttons.SetNum(InMsg.buttons.size());
	for (int32 i = 0; i < InMsg.buttons.size(); ++i)
		Msg.Buttons[i] = InMsg.buttons[i];

	return Msg;
}

FAGX_SensorMsgsJoyFeedback Convert(const agxIO::ROS2::sensorMsgs::JoyFeedback& InMsg)
{
	FAGX_SensorMsgsJoyFeedback Msg;
	Msg.Type = InMsg.type;
	Msg.Id = InMsg.id;
	Msg.Intensity = InMsg.intensity;
	return Msg;
}

FAGX_SensorMsgsJoyFeedbackArray Convert(const agxIO::ROS2::sensorMsgs::JoyFeedbackArray& InMsg)
{
	FAGX_SensorMsgsJoyFeedbackArray Msg;

	Msg.Array.SetNum(InMsg.array.size());
	for (int32 i = 0; i < InMsg.array.size(); ++i)
		Msg.Array[i] = Convert(InMsg.array[i]);

	return Msg;
}

FAGX_SensorMsgsLaserEcho Convert(const agxIO::ROS2::sensorMsgs::LaserEcho& InMsg)
{
	FAGX_SensorMsgsLaserEcho Msg;

	Msg.Echoes.SetNum(InMsg.echoes.size());
	for (int32 i = 0; i < InMsg.echoes.size(); ++i)
		Msg.Echoes[i] = InMsg.echoes[i];

	return Msg;
}

FAGX_SensorMsgsLaserScan Convert(const agxIO::ROS2::sensorMsgs::LaserScan& InMsg)
{
	FAGX_SensorMsgsLaserScan Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.AngleMin = InMsg.angle_min;
	Msg.AngleMax = InMsg.angle_max;
	Msg.AngleIncrement = InMsg.angle_increment;
	Msg.TimeIncrement = InMsg.time_increment;
	Msg.ScanTime = InMsg.scan_time;
	Msg.RangeMin = InMsg.range_min;
	Msg.RangeMax = InMsg.range_max;

	Msg.Ranges.SetNum(InMsg.ranges.size());
	for (int32 i = 0; i < InMsg.ranges.size(); ++i)
		Msg.Ranges[i] = InMsg.ranges[i];

	Msg.Intensities.SetNum(InMsg.intensities.size());
	for (int32 i = 0; i < InMsg.intensities.size(); ++i)
		Msg.Intensities[i] = InMsg.intensities[i];

	return Msg;
}

FAGX_SensorMsgsMagneticField Convert(const agxIO::ROS2::sensorMsgs::MagneticField& InMsg)
{
	FAGX_SensorMsgsMagneticField Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.MagneticField = Convert(InMsg.magnetic_field);

	// Note: This is a dynamic array on the Unreal side, static array on agxROS2 side.
	Msg.MagneticFieldCovariance.SetNum(9);
	for (int32 i = 0; i < 9; ++i)
		Msg.MagneticFieldCovariance[i] = InMsg.magnetic_field_covariance[i];

	return Msg;
}

agxIO::ROS2::stdMsgs::Float32 Convert(const FAGX_StdMsgsFloat32& InMsg)
{
	agxIO::ROS2::stdMsgs::Float32 Msg;
	Msg.data = InMsg.Data;
	return Msg;
}

agxIO::ROS2::stdMsgs::Int32 Convert(const FAGX_StdMsgsInt32& InMsg)
{
	agxIO::ROS2::stdMsgs::Int32 Msg;
	Msg.data = InMsg.Data;
	return Msg;
}
