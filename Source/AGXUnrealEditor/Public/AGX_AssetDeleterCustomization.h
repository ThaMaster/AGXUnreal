

/// @todo Experimental code, do not merge to master.
#pragma message("Experimental code, do not merge to master.")



#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class AGXUNREALEDITOR_API FAGX_AssetDeleterCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
