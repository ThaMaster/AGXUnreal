// Copyright 2022, Algoryx Simulation AB.

#include "NotifyBarrier.h"

#include "BeginAGXIncludes.h"
#include <agx/Referenced.h>
#include "EndAGXIncludes.h"

#include "AGX_LogCategory.h"
#include "AGXRefs.h"

FNotifyBarrier::FNotifyBarrier()
	: NativeRef {new FNotifyRef()}
{
}

FNotifyBarrier::FNotifyBarrier(std::unique_ptr<FNotifyRef> InNativeRef)
	: NativeRef {std::move(InNativeRef)}
{
}

FNotifyBarrier::FNotifyBarrier(FNotifyBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
}

FNotifyBarrier::~FNotifyBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FNotifyRef.
}

bool FNotifyBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

FNotifyRef* FNotifyBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FNotifyRef* FNotifyBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FNotifyBarrier::StartAgxNotify(ELogVerbosity::Type LogVerbosity)
{
	// Note: not allowed to call StartAgxNotify twice in a row.
	check(!HasNative());

	// Allocate native.
	NativeRef->Native = agx::ref_ptr<FAGXNotify>(new FAGXNotify());
	NativeRef->Native->StartAgxNotify(LogVerbosity);
}

void FNotifyBarrier::StopAgxNotify()
{
	check(HasNative());
	NativeRef->Native->StopAgxNotify();

	// Release native.
	NativeRef->Native = nullptr;
}
