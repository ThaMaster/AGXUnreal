#pragma once

// Unreal Engine includes.
#include "IDetailCustomNodeBuilder.h"

class IDetailLayoutBuilder;
class FAGX_WireComponentCustomization;

class FAGX_WireDetailsRuntimeBuilder : public IDetailCustomNodeBuilder,
									   public TSharedFromThis<FAGX_WireDetailsRuntimeBuilder>
{
public:
	FAGX_WireDetailsRuntimeBuilder(
		IDetailLayoutBuilder& InDetailBuilder, FAGX_WireComponentCustomization& InWireDetails);

	//~ Begin IDetailCustomNodeBuilder interface
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;
	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override;
	virtual bool InitiallyCollapsed() const override;
	virtual void SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren) override;
	virtual FName GetName() const override;
	virtual bool RequiresTick() const override;
	virtual void Tick(float DeltaTime) override;
	//~ End IDetailCustomNodeBuilder interface

public:
	void UpdateValues();

public:
	FText OnGetCurrentSpeed() const;
	FText OnGetCurrentPulledInLength() const;
	FText OnGetCurrentMotorForce() const;
	FText OnGetCurrentBrakeForce() const;

public:
	FText RestLength;
	FText CurrentSpeed;
	FText CurrentPulledInLength;
	FText CurrentMotorForce;
	FText CurrentBrakeForce;

public:
	IDetailLayoutBuilder& DetailBuilder;
	FAGX_WireComponentCustomization& WireDetails;
	FSimpleDelegate OnRegenerateChildren;
};
