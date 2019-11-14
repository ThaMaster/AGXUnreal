#include "AGXArchiveStyle.h"
#include "AGXUnrealEditor.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr<FSlateStyleSet> FAGXArchiveStyle::StyleInstance = NULL;

void FAGXArchiveStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FAGXArchiveStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FAGXArchiveStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("AGXArchiveStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BOX_BRUSH(RelativePath, ...) FSlateBoxBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BORDER_BRUSH(RelativePath, ...) FSlateBorderBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define TTF_FONT(RelateivePath, ...) FSlateFontInfo(Style->RootToContentDi(RelateivePath, TEXT(".ttf")), __VA_ARGS__)
#define OTF_FONT(RelateivePath, ...) FSlateFontInfo(Style->RootToContentDir(RelateivePath, TEXT(".otf")), __VA_ARGS__)

const FVector2D Icon16x16(16.f, 16.f);
const FVector2D Icon20x20(20.f, 20.f);
const FVector2D Icon40x40(40.f, 40.f);

TSharedRef<FSlateStyleSet> FAGXArchiveStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet("AGXArchiveStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("AGXUnreal")->GetBaseDir() / TEXT("Resources"));
	Style->Set("AGXArchive.ImportAction", new IMAGE_BRUSH(TEXT("AGXImport_40x"), Icon40x40));
	Style->Set("AGXArchive.ExportAction", new IMAGE_BRUSH(TEXT("AGXExport_40x"), Icon40x40));
	return Style;
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT

void FAGXArchiveStyle::ReloadTextures()
{
	if (!FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FAGXArchiveStyle::Get()
{
	return *StyleInstance;
}
