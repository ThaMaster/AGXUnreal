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
	struct FWireRuntimeState
	{
		FText RestLength;
		FText Mass;
		FText Tension;

		void SetAll(const FText& Text)
		{
			RestLength = Text;
			Mass = Text;
			Tension = Text;
		}
	};

	FWireRuntimeState WireState;

	struct FWinchRuntimeState
	{
		FText Speed;
		FText PulledInLength;
		FText MotorForce;
		FText BrakeForce;

		void SetAll(const FText& Text)
		{
			Speed = Text;
			PulledInLength = Text;
			MotorForce = Text;
			BrakeForce = Text;
		}
	};

	FWinchRuntimeState BeginWinchState;
	FWinchRuntimeState EndWinchState;

public:
	IDetailLayoutBuilder& DetailBuilder;
	FAGX_WireComponentCustomization& WireDetails;
	FSimpleDelegate OnRegenerateChildren;
};
