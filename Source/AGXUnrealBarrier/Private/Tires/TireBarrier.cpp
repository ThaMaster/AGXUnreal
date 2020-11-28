#include "Tires/TireBarrier.h"

// AGXUnreal includes.
#include "AGXRefs.h"


FTireBarrier::FTireBarrier()
	: NativeRef {new FTireRef}
{
}

FTireBarrier::FTireBarrier(std::unique_ptr<FTireRef> Native)
	: NativeRef(std::move(Native))
{
}

FTireBarrier::~FTireBarrier()
{
}

bool FTireBarrier::HasNative() const
{
	return NativeRef && NativeRef->Native;
}

FTireRef* FTireBarrier::GetNative()
{
	return NativeRef.get();
}

const FTireRef* FTireBarrier::GetNative() const
{
	return NativeRef.get();
}

void FTireBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}
