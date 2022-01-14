// Copyright 2022, Algoryx Simulation AB.


#pragma once

#include "Logging/LogVerbosity.h"

#include "BeginAGXIncludes.h"
#include <agx/Notify.h>
#include "EndAGXIncludes.h"

class FAGXNotify : public agx::NotifyCallback
{
public:
	void StartAgxNotify(ELogVerbosity::Type LogVerbosity);
	void StopAgxNotify();

private:
	virtual void message(const agx::String& msg, int notifyLevel) override;
};
