// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AGX_AgxEdModeSubMode.generated.h"

/**
 * Base class for all sub-modes for AgxEdMode.
 *
 * Each sub-mode gets a toolbar entry in the submode toolbar.
 * The Detail View of the submode object fills the AgxEdMode
 * panel below the toolbar when the toolbar entry is clicked.
 */
UCLASS(abstract, ClassGroup = "AGX", Category = "AGX", config = EditorPerProjectUserSettings)
class AGXUNREALEDITOR_API UAGX_AgxEdModeSubMode : public UObject
{
	GENERATED_BODY()

public:

	virtual FText GetDisplayName() const PURE_VIRTUAL(AAGX_Constraint::CreateNativeImpl, return FText(); );

	virtual FText GetTooltip() const PURE_VIRTUAL(AAGX_Constraint::CreateNativeImpl, return FText(); );
};
