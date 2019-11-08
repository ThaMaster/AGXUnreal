// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


/**
 * Style that can be used by Editor-only AGX objects.
 */
class AGXUNREALEDITOR_API FAGX_EditorStyle
{
public: // Names of common resources

	static const FName AgxIcon;
	static const FName AgxIconSmall;

public:

	static void Initialize();
	static void Shutdown();
	static void ReloadTextures();
	static TSharedPtr<class ISlateStyle> Get();
	static FName GetStyleSetName();

private:

	static TSharedRef<class FSlateStyleSet> Create();

private:

	static TSharedPtr<class FSlateStyleSet> StyleInstance;

};
