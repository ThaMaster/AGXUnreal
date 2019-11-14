// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AgxEdMode/AGX_AgxEdModeSubMode.h"
#include "AGX_AgxEdModeFile.generated.h"

/**
 * Sub-mode for AgxEdMode. Used to import/export .agx files.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", config = EditorPerProjectUserSettings)
class AGXUNREALEDITOR_API UAGX_AgxEdModeFile : public UAGX_AgxEdModeSubMode
{
	GENERATED_BODY()

public:

	static UAGX_AgxEdModeFile* GetInstance();

public:

	virtual FText GetDisplayName() const override;
	virtual FText GetTooltip() const override;

public: // AGX File Importer

	UPROPERTY(EditAnywhere, Category = "AGX File Importer")
	int DummyPlaceholder; /// \todo Replace with actual properties.

public: // AGX File Exporter

	UPROPERTY(EditAnywhere, Category = "AGX File Exporter")
	int DummyPlaceholder2; /// \todo Replace with actual properties.

};
