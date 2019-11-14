// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AgxEdMode/AGX_AgxEdModeSubMode.h"
#include "AGX_AgxEdModeBodies.generated.h"

/**
 * Sub-mode for AgxEdMode. Used to create and manage constraints.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", config = EditorPerProjectUserSettings)
class AGXUNREALEDITOR_API UAGX_AgxEdModeBodies : public UAGX_AgxEdModeSubMode
{
	GENERATED_BODY()

public:
	
	static UAGX_AgxEdModeBodies* GetInstance();

public:

	virtual FText GetDisplayName() const override;
	virtual FText GetTooltip() const override;

public: // Rigid Body Creator

	UPROPERTY(EditAnywhere, Category = "Rigid Body Creator")
	int DummyPlaceholder; /// \todo Replace with actual properties.

public: // Rigid Body Browser

	UPROPERTY(EditAnywhere, Category = "Rigid Body Browser")
	int DummyPlaceholder2; /// \todo Replace with actual properties.

};
