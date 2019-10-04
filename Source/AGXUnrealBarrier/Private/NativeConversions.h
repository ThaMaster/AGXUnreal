#pragma once


/**
 * Given a Barrier, returns the final AGX native object.
 */
template<typename TNative, typename TBarrier>
TNative* GetNativeFromBarrier(const TBarrier* Barrier)
{
	if (Barrier || Barrier->HasNative())
		return Barrier->GetNative()->Native.get();
	else
		return nullptr;
}
