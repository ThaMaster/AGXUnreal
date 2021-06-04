#include "NativeBarrier.h"

// AGX Dynamics for Unreal includes.
#include <AGX_LogCategory.h>

template <typename FNativeRef>
FNativeBarrier<FNativeRef>::FNativeBarrier()
	: NativeRef {new FNativeRef}
{
}

template <typename FNativeRef>
FNativeBarrier<FNativeRef>::FNativeBarrier(std::unique_ptr<FNativeRef> Native)
	: NativeRef {std::move(Native)}
{
	check(NativeRef);
}

template <typename FNativeRef>
FNativeBarrier<FNativeRef>::FNativeBarrier(FNativeBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FNativeRef);
}

template <typename FNativeRef>
FNativeBarrier<FNativeRef>::~FNativeBarrier()
{
	// Must provide a destructor implementation in the implementation file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition, not just the
	// forward declaration, of FNativeRef.
}

template <typename FNativeRef>
bool FNativeBarrier<FNativeRef>::HasNative() const
{
	check(NativeRef);
	return NativeRef->Native != nullptr;
}

// See comment on member function declaration in the header file. Remove this if the declaration
// has been removed.
#if 0
template <typename FNativeRef>
void FNativeBarrier<FNativeRef>::AllocateNative()
{
	check(!HasNative());
	PreNativeChanged();
	DoAllocateNative();
	PostNativeChanged();
}
#endif

template <typename FNativeRef>
FNativeRef* FNativeBarrier<FNativeRef>::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

template <typename FNativeRef>
const FNativeRef* FNativeBarrier<FNativeRef>::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

template <typename FNativeRef>
uintptr_t FNativeBarrier<FNativeRef>::GetNativeAddress() const
{
	if (!HasNative())
	{
		return 0;
	}
	return reinterpret_cast<uintptr_t>(NativeRef->Native.get());
}

template <typename FNativeRef>
void FNativeBarrier<FNativeRef>::SetNativeAddress(uintptr_t NativeAddress)
{
	if (NativeAddress == GetNativeAddress())
	{
		return;
	}

	PreNativeChanged();

	if (HasNative())
	{
		ReleaseNative();
	}

	if (NativeAddress == 0)
	{
		NativeRef->Native = nullptr;
	}
	else
	{
		NativeRef->Native = reinterpret_cast<typename FNativeRef::NativeType*>(NativeAddress);
	}

	PostNativeChanged();
}

template <typename FNativeRef>
void FNativeBarrier<FNativeRef>::ReleaseNative()
{
	PreNativeChanged();
	NativeRef->Native = nullptr;
	PostNativeChanged();
}
