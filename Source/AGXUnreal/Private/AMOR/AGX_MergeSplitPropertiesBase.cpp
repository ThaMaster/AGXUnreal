// Copyright 2022, Algoryx Simulation AB.

#include "AMOR/AGX_MergeSplitPropertiesBase.h"


FAGX_MergeSplitPropertiesBase& FAGX_MergeSplitPropertiesBase::operator=(
	const FAGX_MergeSplitPropertiesBase& Other)
{
	bEnableMerge = Other.bEnableMerge;
	bEnableSplit = Other.bEnableSplit;
	return *this;
}

void FAGX_MergeSplitPropertiesBase::SetEnableMerge(bool bEnable)
{
	bEnableMerge = bEnable;
	if (HasNative())
	{
		NativeBarrier.SetEnableMerge(bEnable);
	}
}

bool FAGX_MergeSplitPropertiesBase::GetEnableMerge() const
{
	return bEnableMerge;
}

void FAGX_MergeSplitPropertiesBase::SetEnableSplit(bool bEnable)
{
	bEnableSplit = bEnable;
	if (HasNative())
	{
		NativeBarrier.SetEnableSplit(bEnable);
	}
}

bool FAGX_MergeSplitPropertiesBase::GetEnableSplit() const
{
	return bEnableSplit;
}

bool FAGX_MergeSplitPropertiesBase::HasNative() const
{
	return NativeBarrier.HasNative();
}

const FMergeSplitPropertiesBarrier& FAGX_MergeSplitPropertiesBase::GetNative() const
{
	return NativeBarrier;
}

FMergeSplitPropertiesBarrier& FAGX_MergeSplitPropertiesBase::GetNative()
{
	return NativeBarrier;
}

void FAGX_MergeSplitPropertiesBase::CopyFrom(const FMergeSplitPropertiesBarrier& Barrier)
{
	bEnableMerge = Barrier.GetEnableMerge();
	bEnableSplit = Barrier.GetEnableSplit();
}