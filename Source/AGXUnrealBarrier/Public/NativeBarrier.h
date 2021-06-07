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

	/// @todo This can't exist because the parameter list is different.
	// void AllocateNative();

	FNativeRef* GetNative();
	const FNativeRef* GetNative() const;
	uintptr_t GetNativeAddress() const;
	void SetNativeAddress(uintptr_t NativeAddress);

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
