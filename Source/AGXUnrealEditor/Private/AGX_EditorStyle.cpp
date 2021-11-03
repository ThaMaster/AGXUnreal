#include "AGX_EditorStyle.h"

#include "Runtime/Projects/Public/Interfaces/IPluginManager.h"
#include "Runtime/SlateCore/Public/Styling/SlateStyle.h"
#include "Runtime/SlateCore/Public/Styling/SlateStyleRegistry.h"
#include "Runtime/SlateCore/Public/Styling/SlateTypes.h"
#include "SlateOptMacros.h"

TSharedPtr<FSlateStyleSet> FAGX_EditorStyle::StyleInstance = nullptr;
const FName FAGX_EditorStyle::AgxIcon("AgxIcon");
const FName FAGX_EditorStyle::AgxIconSmall("AgxIcon.Small");
const FName FAGX_EditorStyle::JointIcon("JointIcon");
const FName FAGX_EditorStyle::JointIconSmall("JointIcon.Small");

void FAGX_EditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FAGX_EditorStyle::Shutdown()
{
	if (StyleInstance.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance.Get());
		ensure(StyleInstance.IsUnique());
		StyleInstance.Reset();
	}
}

void FAGX_EditorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

TSharedPtr<class ISlateStyle> FAGX_EditorStyle::Get()
{
	return StyleInstance;
}

FName FAGX_EditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("AGX_EditorStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH(RelativePath, ...) \
	FSlateImageBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BOX_BRUSH(RelativePath, ...) \
	FSlateBoxBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BORDER_BRUSH(RelativePath, ...) \
	FSlateBorderBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define TTF_FONT(RelateivePath, ...) \
	FSlateFontInfo(Style->RootToContentDi(RelateivePath, TEXT(".ttf")), __VA_ARGS__)
#define OTF_FONT(RelateivePath, ...) \
	FSlateFontInfo(Style->RootToContentDir(RelateivePath, TEXT(".otf")), __VA_ARGS__)

namespace
{
	const FVector2D IconSize16(16.0f, 16.0f);
	const FVector2D IconSize40(40.0f, 40.0f);
	const FVector2D IconSize128(128.0f, 128.0f);
}

TSharedRef<class FSlateStyleSet> FAGX_EditorStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	Style->SetContentRoot(
		IPluginManager::Get().FindPlugin("AGXUnreal")->GetContentDir() / TEXT("Editor"));

	// Define icons and stuff here.

	Style->Set(AgxIcon, new IMAGE_BRUSH("Icons/T_Icon_A_Light_128", IconSize40));
	Style->Set(AgxIconSmall, new IMAGE_BRUSH("Icons/T_Icon_A_Light_776", IconSize16));
	Style->Set(JointIcon, new IMAGE_BRUSH("Icons/T_Icon_Constraint_200", IconSize40));
	Style->Set(JointIconSmall, new IMAGE_BRUSH("Icons/T_Icon_Constraint_200", IconSize16));
	Style->Set("ClassIcon.AGX_RigidBodyComponent", new IMAGE_BRUSH("Icons/T_Icon_A_Light_776", IconSize16));
	Style->Set("ClassIcon.AGX_RigidBodyActor", new IMAGE_BRUSH("Icons/T_Icon_A_Light_776", IconSize16));
	Style->Set("ClassIcon.AGX_BoxShapeComponent", new IMAGE_BRUSH("Icons/T_Icon_Box_200", IconSize16));
	Style->Set("ClassIcon.AGX_CylinderShapeComponent", new IMAGE_BRUSH("Icons/T_Icon_Cylinder_200", IconSize16));
	Style->Set("ClassIcon.AGX_CapsuleShapeComponent", new IMAGE_BRUSH("Icons/T_Icon_Capsule_200", IconSize16));

	return Style;
};

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT
