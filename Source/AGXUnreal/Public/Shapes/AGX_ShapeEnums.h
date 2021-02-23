#pragma once

UENUM(BlueprintType)
enum EAGX_ShapeSensorType
{
	/** This shape will generate contact point information. Note that this is alternative is more
	 * computationally expensive than the Boolean Sensor setting.
	 */
	ContactsSensor,

	/** This shape will not generate contact point information, but will detect if this shape is in
	 * contact with other shapes. This alternative may be used to increase performance where contact
	 * point data is not needed.
	 */
	BooleanSensor
};
