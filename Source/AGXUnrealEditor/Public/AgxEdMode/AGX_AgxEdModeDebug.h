#pragma once

#include "CoreMinimal.h"
#include "AgxEdMode/AGX_AgxEdModeSubMode.h"
#include "AGX_AgxEdModeDebug.generated.h"

/**
 * Sub-mode for AgxEdMode. Used to create and manage constraints.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", config = EditorPerProjectUserSettings)
class AGXUNREALEDITOR_API UAGX_AgxEdModeDebug : public UAGX_AgxEdModeSubMode
{
	GENERATED_BODY()

public:
	static UAGX_AgxEdModeDebug* GetInstance();

public:
	virtual FText GetDisplayName() const override;
	virtual FText GetTooltip() const override;

public: // Debug Visualizer
	UPROPERTY(EditAnywhere, Category = "Debug Visualizer")
	int DummyPlaceholder; /// \todo Replace with actual properties.

public: // Logger
	UPROPERTY(EditAnywhere, Category = "Logger")
	int DummyPlaceholder2; /// \todo Replace with actual properties.
};
