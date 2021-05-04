#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

struct FSuccessOrError
{
	bool Success;
	FString Error;

	explicit FSuccessOrError(bool InSuccess)
		: Success(InSuccess)
	{
	}

	explicit FSuccessOrError(FString InError)
		: Success(false)
		, Error(std::move(InError))
	{
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
