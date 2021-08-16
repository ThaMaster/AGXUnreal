#include "Wire/AGX_WireDetailsRuntimeBuilder.h"

// AGX Dynamics for Unreal includes.
#include "Wire/AGX_WireComponentCustomization.h"

// Unreal Engine includes.
#include "IDetailChildrenBuilder.h"
#include "IDetailGroup.h"
#include "DetailWidgetRow.h"

#define LOCTEXT_NAMESPACE "WireDetailsRuntimeBuilder"

FAGX_WireDetailsRuntimeBuilder::FAGX_WireDetailsRuntimeBuilder(
	IDetailLayoutBuilder& InDetailBuilder, FAGX_WireComponentCustomization& InWireDetails)
	: DetailBuilder(InDetailBuilder)
	, WireDetails(InWireDetails)
{
	UpdateValues();
}

void FAGX_WireDetailsRuntimeBuilder::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
	// By having an empty header row Slate won't generate a collapsable section for the node
	// details.
	/// @todo Maybe we do want one in this case?
}

namespace AGX_WireDetailsRuntimeBuilder_helpers
{
	void CreateRuntimeDisplay(
		FAGX_WireDetailsRuntimeBuilder& Details, IDetailChildrenBuilder& ChildrenBuilder,
		const FText& Name, FText (FAGX_WireDetailsRuntimeBuilder::*OnGet)() const)
	{
		// clang-format off
		ChildrenBuilder.AddCustomRow(Name)
		.Visibility(TAttribute<EVisibility>(
			&Details.WireDetails, &FAGX_WireComponentCustomization::WithNative))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(Name)
		]
		.ValueContent()
		[
			SNew(STextBlock)
			.Text(&Details, OnGet)
		];
		// clang-format on
	}

	template<typename FOnGet>
	void CreateRuntimeDisplay(
		FAGX_WireDetailsRuntimeBuilder& Details, IDetailChildrenBuilder& ChildrenBuilder,
		const FText& Name, FOnGet OnGet)
	{
		// clang-format off
		ChildrenBuilder.AddCustomRow(Name)
		.Visibility(TAttribute<EVisibility>(
			&Details.WireDetails, &FAGX_WireComponentCustomization::WithNative))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(Name)
		]
		.ValueContent()
		[
			SNew(STextBlock)
			.Text_Lambda(OnGet)
		];
		// clang-format on
	}

	template<typename FOnGet>
	void CreateRuntimeDisplay(
		FAGX_WireDetailsRuntimeBuilder& Details, IDetailGroup& Group, const FText& Name, FOnGet OnGet)
	{
		// clang-format off
		Group.AddWidgetRow()
		.Visibility(TAttribute<EVisibility>(
			&Details.WireDetails, &FAGX_WireComponentCustomization::WithNative))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(Name)
		]
		.ValueContent()
		[
			SNew(STextBlock)
			.Text_Lambda(OnGet)
		];
		// clang-format on
	}
}

void FAGX_WireDetailsRuntimeBuilder::GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder)
{
	using namespace AGX_WireDetailsRuntimeBuilder_helpers;

	IDetailGroup& WireGroup = ChildrenBuilder.AddGroup(TEXT("Wire"), LOCTEXT("Wire", "Wire"));
	IDetailGroup& BeginWinchGroup =
		ChildrenBuilder.AddGroup(TEXT("Begin Winch"), LOCTEXT("BeginWinch", "Begin Winch"));
	IDetailGroup& EndWinchGroup =
		ChildrenBuilder.AddGroup(TEXT("End Winch"), LOCTEXT("EndWinch", "End Winch"));

	/* Wire. */
	CreateRuntimeDisplay(
		*this, WireGroup, LOCTEXT("RestLength", "Rest Length"),
		[this]() { return WireState.RestLength; });

	CreateRuntimeDisplay(
		*this, WireGroup, LOCTEXT("Mass", "Mass"),
		[this]() { return WireState.Mass; });

	CreateRuntimeDisplay(
		*this, WireGroup, LOCTEXT("Tension", "Tension"),
		[this]() { return WireState.Tension; });

	/* Begin winch. */

	CreateRuntimeDisplay(
		*this, BeginWinchGroup, LOCTEXT("CurrentSpeed", "Current Speed"),
		[this]() { return BeginWinchState.Speed; });

	CreateRuntimeDisplay(
		*this, BeginWinchGroup, LOCTEXT("PulledInLength", "Pulled In Length"),
		[this]() { return BeginWinchState.PulledInLength; });

	CreateRuntimeDisplay(
		*this, BeginWinchGroup, LOCTEXT("MotorForce", "Motor Force"),
		[this]() { return BeginWinchState.MotorForce; });

	CreateRuntimeDisplay(
		*this, BeginWinchGroup, LOCTEXT("BrakeForce", "Brake Force"),
		[this]() { return BeginWinchState.BrakeForce; });

	/* End winch. */

	CreateRuntimeDisplay(
		*this, EndWinchGroup, LOCTEXT("CurrentSpeed", "Current Speed"),
		[this]() { return EndWinchState.Speed; });

	CreateRuntimeDisplay(
		*this, EndWinchGroup, LOCTEXT("PulledInLength", "Pulled In Length"),
		[this]() { return EndWinchState.PulledInLength; });

	CreateRuntimeDisplay(
		*this, EndWinchGroup, LOCTEXT("MotorForce", "Motor Force"),
		[this]() { return EndWinchState.MotorForce; });

	CreateRuntimeDisplay(
		*this, EndWinchGroup, LOCTEXT("BrakeForce", "Brake Force"),
		[this]() { return EndWinchState.BrakeForce; });
}

bool FAGX_WireDetailsRuntimeBuilder::InitiallyCollapsed() const
{
	return false;
}

void FAGX_WireDetailsRuntimeBuilder::SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren)
{
	OnRegenerateChildren = InOnRegenerateChildren;
}

FName FAGX_WireDetailsRuntimeBuilder::GetName() const
{
	return TEXT("Wire Runtime");
}

bool FAGX_WireDetailsRuntimeBuilder::RequiresTick() const
{
	return true;
}

void FAGX_WireDetailsRuntimeBuilder::Tick(float DeltaTime)
{
	UpdateValues();
}

void FAGX_WireDetailsRuntimeBuilder::UpdateValues()
{
	static const FText NoWire = LOCTEXT("NoWire", "No Wire");
	static const FText NoNative = LOCTEXT("NoNative", "No Native");

	UAGX_WireComponent* Wire = WireDetails.Wire.Get();
	if (Wire == nullptr)
	{
		WireState.SetAll(NoWire);
		BeginWinchState.SetAll(NoWire);
		EndWinchState.SetAll(NoWire);
		return;
	}

	if (Wire->HasNative())
	{
		WireState.RestLength = FText::Format(
			LOCTEXT("RestLengthValue", "{0} cm"), FText::AsNumber(Wire->GetRestLength()));

		WireState.Mass = FText::Format(
			 LOCTEXT("MassValue", "{0} kg"), FText::AsNumber(Wire->GetMass()));

		WireState.Tension = FText::Format(
			LOCTEXT("TensionValue", "{0} N"), FText::AsNumber(Wire->GetTension()));
	}
	else
	{
		WireState.SetAll(NoNative);
	}

	if (Wire->OwnedBeginWinch.HasNative())
	{
		// We typically don't keep the entire Unreal Engine state up-to-date with the AGX Dynamics
		// objects at all time. Here, however, the user is actively looking at a particular Wire and
		// its winches so it makes sense to update them.
		//
		// To avoid the GUI having unintended side-effects on the state
		Wire->OwnedBeginWinch.ReadPropertiesFromNative();

		BeginWinchState.Speed = FText::Format(
			LOCTEXT("SpeedValue", "{0} cm/s"),
			FText::AsNumber(Wire->OwnedBeginWinch.GetCurrentSpeed()));
		BeginWinchState.PulledInLength = FText::Format(
			LOCTEXT("PulledInLengthValue", "{0} cm"),
			FText::AsNumber(Wire->OwnedBeginWinch.GetPulledInLength()));
		BeginWinchState.MotorForce = FText::Format(
			LOCTEXT("MotorForceValue", "{0} N"),
			FText::AsNumber(Wire->OwnedBeginWinch.GetCurrentMotorForce()));
		BeginWinchState.BrakeForce = FText::Format(
			LOCTEXT("BrakeForceValue", "{0} N"),
			FText::AsNumber(Wire->OwnedBeginWinch.GetCurrentBrakeForce()));
	}
	else
	{
		BeginWinchState.SetAll(NoNative);
	}

	/// @TODO This is copy/paste from BeginWinch. Move to a helper function.
	if (Wire->OwnedEndWinch.HasNative())
	{
		// We typically don't keep the entire Unreal Engine state up-to-date with the AGX Dynamics
		// objects at all time. Here, however, the user is actively looking at a particular Wire and
		// its winches so it makes sense to update them.
		//
		// To avoid the GUI having unintended side-effects on the state
		Wire->OwnedEndWinch.ReadPropertiesFromNative();

		EndWinchState.Speed = FText::Format(
			LOCTEXT("SpeedValue", "{0} cm/s"),
			FText::AsNumber(Wire->OwnedEndWinch.GetCurrentSpeed()));
		EndWinchState.PulledInLength = FText::Format(
			LOCTEXT("PulledInLengthValue", "{0} cm"),
			FText::AsNumber(Wire->OwnedEndWinch.GetPulledInLength()));
		EndWinchState.MotorForce = FText::Format(
			LOCTEXT("MotorForceValue", "{0} N"),
			FText::AsNumber(Wire->OwnedEndWinch.GetCurrentMotorForce()));
		EndWinchState.BrakeForce = FText::Format(
			LOCTEXT("BrakeForceValue", "{0} N"),
			FText::AsNumber(Wire->OwnedEndWinch.GetCurrentBrakeForce()));
	}
	else
	{
		EndWinchState.SetAll(NoNative);
	}


}

#undef LOCTEXT_NAMESPACE
