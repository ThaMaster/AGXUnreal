#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_MeshWithTransform.h"

// Unreal Engine includes.
#include "IDetailCustomNodeBuilder.h"
#include "SceneOutlinerFilters.h"
#include "Widgets/Input/SComboBox.h"

class IDetailLayoutBuilder;
class UStaticMesh;

UENUM()
enum EAGX_MeshLocation
{
	AnyChildren,
	ImmediateChildren,
	Parent,
	Asset
};

struct FAutoFitMeshLocation
{
	FAutoFitMeshLocation(
		const FString& InName, const FString& InToolTip, enum EAGX_MeshLocation InMeshLocation)
		: Name(InName)
		, ToolTip(InToolTip)
		, MeshLocation(InMeshLocation)
	{
	}

	const FString Name;
	const FString ToolTip;
	const enum EAGX_MeshLocation MeshLocation;
};

class FAGX_AutoFitShapeDetails : public IDetailCustomNodeBuilder,
								 public TSharedFromThis<FAGX_AutoFitShapeDetails>
{
public:
	FAGX_AutoFitShapeDetails(IDetailLayoutBuilder& InDetailBuilder);

	//~ Begin IDetailCustomNodeBuilder.
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;
	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override;
	virtual bool InitiallyCollapsed() const override;
	virtual void SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren) override;
	virtual FName GetName() const override;
	virtual bool RequiresTick() const override;
	virtual void Tick(float DeltaTime) override;
	//~ End IDetailCustomNodeBuilder.

protected:
	void OnAssetSelected(const FAssetData& AssetData);
	FString GetCurrentAssetPath() const;

private:
	FReply OnAutoFitButtonClicked();
	void OnMeshLocationComboBoxChanged(
		TSharedPtr<FAutoFitMeshLocation> NewMeshLocation, ESelectInfo::Type InSeletionInfo);

	EVisibility GetAssetPickerVisibility() const;
	UStaticMesh* GetSelectedStaticMeshAsset() const;
	TArray<FAGX_MeshWithTransform> GetSelectedStaticMeshes(USceneComponent* Shape) const;

	TArray<TSharedPtr<FAutoFitMeshLocation>> MeshLocations;
	TSharedPtr<FAutoFitMeshLocation> CurrentlySelectedMeshLocation;
	FAssetData CurrentlySelectedAsset;
	IDetailLayoutBuilder& DetailBuilder;
};
