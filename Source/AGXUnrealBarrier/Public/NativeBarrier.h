// Copyright 2022, Algoryx Simulation AB.

#pragma once

// System includes.
#include <cstdint>
#include <memory>

template <typename FNativeRef>
class FNativeBarrier
{
public:
	FNativeBarrier();
	FNativeBarrier(std::unique_ptr<FNativeRef> Native);
	FNativeBarrier(FNativeBarrier&& Other);
	virtual ~FNativeBarrier();

	bool HasNative() const;
	FNativeRef* GetNative();
	const FNativeRef* GetNative() const;
	uintptr_t GetNativeAddress() const;
	void SetNativeAddress(uintptr_t NativeAddress);

	/**
	 * Increment the reference count of the AGX Dynamics object. This should always be paired with
	 * a call to DecrementRefCount, and the count should only be artificially incremented for a
	 * very well specified duration.
	 *
	 * One use-case is during a Blueprint reconstruction, when the Unreal Engine objects are
	 * destroyed and then recreated. During this time the AGX Dynamics objects are retained and
	 * handed between the old and the new Unreal Engine objects through a Component Instance Data.
	 * This Component Instance Data instance is considered the owner of the AGX Dynamics object
	 * during this transition period and the reference count is therefore increment during its
	 * lifetime. We're lending out ownership of the AGX Dynamics object to the Component Instance
	 * Data instance for the duration of the Blueprint reconstruction.
	 *
	 * These functions can be const even though they have observable side effects because the
	 * reference count is not a salient part of the AGX Dynamics objects, and they are thread-safe.
	 */
	void IncrementRefCount() const;
	void DecrementRefCount() const;

	/**
	 * Set the agx::ref_ptr owned by this Native Barrier to nullptr, causing the AGX Dynamics object
	 * to be deleted if this was the last owner.
	 */
	void ReleaseNative();

protected:
	// May need a ENativeChangedReason here.
	virtual void PreNativeChanged() {};
	virtual void PostNativeChanged() {};

private:
	FNativeBarrier(const FNativeBarrier&) = delete;
	void operator=(const FNativeBarrier&) = delete;

protected:
	std::unique_ptr<FNativeRef> NativeRef;
};
