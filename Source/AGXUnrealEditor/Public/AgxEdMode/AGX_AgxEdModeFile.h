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

public:
	static void ImportAgxArchiveToLevel();
	static void ImportAgxArchiveToBlueprint();
	static void ExportAgxArchive();
};
