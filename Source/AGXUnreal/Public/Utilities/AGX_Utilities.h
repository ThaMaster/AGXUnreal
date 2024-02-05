// Copyright 2024, Algoryx Simulation AB.

#pragma once

/**
 * Helper type that calls the given callback when destructed. Useful when cleanup is needed
 * regardless of from where we return from a function or leave a scope.
 * @tparam FuncT Callback to call on destruction. Will be moved from.
 */
template <typename FuncT>
struct FAGX_Finalizer
{
	FAGX_Finalizer(FuncT InCallback)
		: Callback(std::move(InCallback))
	{
	}

	~FAGX_Finalizer()
	{
		Callback();
	}

	FuncT Callback;
};
