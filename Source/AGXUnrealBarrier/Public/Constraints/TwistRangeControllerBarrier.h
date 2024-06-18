// Copyright 2024, Algoryx Simulation AB.

#pragma once

#include <memory>

#if 0

struct FTwistRangeControllerRef;

class AGXUNREALBARRIER_API FTwistRangeControllerBarrier
{
public: // Special member functions.
	/// Create a Barrier with a Ref but without a Native.
	FTwistRangeControllerBarrier();
	/// Create a Barrier with the same Native as Other, but a separate Ref.
	FTwistRangeControllerBarrier(const FTwistRangeControllerBarrier& Other);
	/// Create a Barrier that takes over ownership of the given Ref.
	FTwistRangeControllerBarrier(std::unique_ptr<FTwistRangeControllerRef> Native);
	/**
	 * Destructor must be virtual since we (will) inherit and must be non-inline because the
	 * Ref type definition isn't known here so the std::unique_ptr destructor cannot be
	 * instantiated.
	 */
	virtual ~FTwistRangeControllerBarrier();

	/**
	 * Make this Barrier point to the same Native object as Other. The Ref object will not be
	 * shared, only the AGX Dynamics object.
	 *
	 * If this was the last reference to the current Native object then it will be deleted.
	 */
	FTwistRangeControllerBarrier& operator=(const FTwistRangeControllerBarrier& Other);

public: // AGX Dynamics accessors.
	void SetEnabled(bool bInEnabled);
	bool GetEnabled() const;

	void SetRange(FDoubleInterval InRange);
	FDoubleInterval GetRange() const;

public: // Native management.
	bool HasNative() const;
	FTwistRangeControllerRef* GetNative();
	const FTwistRangeControllerRef* GetNative() const;

protected:
	std::unique_ptr<FTwistRangeControllerRef> NativeRef;
};

#else

struct FTwistRangeControllerRef;

class AGXUNREALBARRIER_API FTwistRangeControllerBarrier
{
public: // Special member functions.
	FTwistRangeControllerBarrier();
	FTwistRangeControllerBarrier(const FTwistRangeControllerBarrier& Other);
	FTwistRangeControllerBarrier(std::unique_ptr<FTwistRangeControllerRef> InNative);
	virtual ~FTwistRangeControllerBarrier();
	FTwistRangeControllerBarrier& operator=(const FTwistRangeControllerBarrier& Other);

public: // AGX Dynamics accessors.
	void SetEnabled(bool bInEnabled);
	bool GetEnabled() const;

	void SetRange(FDoubleInterval InRange);
	FDoubleInterval GetRange() const;

public: // Native management.
	bool HasNative() const;
	FTwistRangeControllerRef* GetNative();
	const FTwistRangeControllerRef* GetNative() const;


private: // Member variables.
	std::unique_ptr<FTwistRangeControllerRef> NativeRef;
};

#endif
