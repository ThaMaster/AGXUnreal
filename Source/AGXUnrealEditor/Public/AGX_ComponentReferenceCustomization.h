// Copyright 2023, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"

struct FAGX_ComponentReference;

class FAGX_ComponentReferenceCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	// ~Begin IPropertyTypeCustomization interface.
	virtual void CustomizeHeader(
		TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(
		TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	// ~End IPropertyTypeCustomization interface.

private:
	FText GetHeaderText() const;

	void RebuildComboBox();

	void OnComboBoxChanged(TSharedPtr<FName> NewSelection, ESelectInfo::Type SelectionInfo);
	void OnComboBoxCommitted(const FText& Text, ETextCommit::Type CommitType);
	void OnComponentNameCommitted(const FText& InText, ETextCommit::Type InCommitType);

	TArray<TSharedPtr<FName>> ComponentNames;

	FName SelectedComponent;

	SComboBox<TSharedPtr<FName>>* ComboBoxPtr = nullptr;

	FSimpleDelegate RebuildComboBoxDelegate;

	SEditableTextBox* ComponentNameBoxPtr = nullptr;

	/**
	 * Re-fetch the pointers/handles to the underlying data stores.
	 * This should be called at the start of every Customize.+ function.
	 */
	void RefreshStoreReferences(const TSharedRef<IPropertyHandle>& InComponentReferenceHandle);

	FAGX_ComponentReference* GetComponentReference() const;

	AActor* GetOwningActor() const;

	TSharedPtr<IPropertyHandle> ComponentReferenceHandle;
	TSharedPtr<IPropertyHandle> OwningActorHandle;
	TSharedPtr<IPropertyHandle> NameHandle;
	TSharedPtr<IPropertyHandle> SearchChildActorsHandle;
};
