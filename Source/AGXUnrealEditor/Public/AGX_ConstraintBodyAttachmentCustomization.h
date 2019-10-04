// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Layout/Visibility.h"
#include "IPropertyTypeCustomization.h"
#include "PropertyHandle.h"


class FAGX_ConstraintBodyAttachmentCustomization : public IPropertyTypeCustomization
{
public:

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
	
	virtual void CustomizeHeader(
		TSharedRef<class IPropertyHandle> StructPropertyHandle, 
		class FDetailWidgetRow& HeaderRow, 
		IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

	virtual void CustomizeChildren(
		TSharedRef<class IPropertyHandle> StructPropertyHandle,
		class IDetailChildrenBuilder& StructBuilder,
		IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

	FText GetRigidBodyName() const;

	bool HasFrameDefiningActor() const;

	void CreateAndSetFrameDefiningActor();

	static AActor* CreateFrameDefiningActor(class AAGX_Constraint* Constraint, class AActor* RigidBody);

private:

	TSharedPtr<class IPropertyHandle> BodyAttachmentProperty = nullptr;
	TSharedPtr<class IPropertyHandle> RigidBodyProperty = nullptr;
	TSharedPtr<class IPropertyHandle> FrameDefiningActorProperty = nullptr;
};