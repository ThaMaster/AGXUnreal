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

void FNotifyBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->Native = agx::ref_ptr<FAGXNotify>(new FAGXNotify());
	UE_LOG(LogAGX, Log, TEXT("Native AGXNotify allocated."));
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
	check(HasNative());
	NativeRef->Native->StartAgxNotify(LogVerbosity);
}

void FNotifyBarrier::StopAgxNotify()
{
	check(HasNative());
	NativeRef->Native->StopAgxNotify();
}
