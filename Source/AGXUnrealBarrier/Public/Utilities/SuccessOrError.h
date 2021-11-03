#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

struct FSuccessOrError
{
	bool Success;
	bool HasWarning;
	FString Error;
	FString Warning;

	explicit FSuccessOrError(bool InSuccess)
		: Success(InSuccess)
		, HasWarning(false)
	{
	}

	explicit FSuccessOrError(FString InError)
		: Success(false)
		, HasWarning(false)
		, Error(std::move(InError))
	{
	}

	void AddWarning(const FString& InWarning)
	{
		HasWarning = true;
		Warning = InWarning;
	}

	operator bool() const
	{
		return Success;
	}

	bool IsSuccess() const
	{
		return Success;
	}
};
