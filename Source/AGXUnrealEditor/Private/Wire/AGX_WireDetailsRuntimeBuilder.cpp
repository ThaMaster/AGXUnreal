#include "Wire/AGX_WireDetailsRuntimeBuilder.h"

// AGX Dynamics for Unreal includes.
#include "Wire/AGX_WireComponentCustomization.h"

// Unreal Engine includes.
#include "IDetailChildrenBuilder.h"
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
}

void FAGX_WireDetailsRuntimeBuilder::GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder)
{
	using namespace AGX_WireDetailsRuntimeBuilder_helpers;

	CreateRuntimeDisplay(
		*this, ChildrenBuilder, LOCTEXT("RestLength", "RestLength"),
		[this]() { return RestLength; });

	CreateRuntimeDisplay(
		*this, ChildrenBuilder, LOCTEXT("CurrentSpeed", "CurrentSpeed"),
		[this]() { return CurrentSpeed; });

	CreateRuntimeDisplay(
		*this, ChildrenBuilder, LOCTEXT("PulledInLength", "Pulled In Length"),
		&FAGX_WireDetailsRuntimeBuilder::OnGetCurrentPulledInLength);

	CreateRuntimeDisplay(
		*this, ChildrenBuilder, LOCTEXT("MotorForce", "Motor Force"),
		&FAGX_WireDetailsRuntimeBuilder::OnGetCurrentMotorForce);

	CreateRuntimeDisplay(
		*this, ChildrenBuilder, LOCTEXT("BrakeForce", "Brake Force"),
		&FAGX_WireDetailsRuntimeBuilder::OnGetCurrentBrakeForce);
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
		RestLength = NoWire;
		CurrentSpeed = NoWire;
		CurrentPulledInLength = NoWire;
		CurrentMotorForce = NoWire;
		CurrentBrakeForce = NoWire;
		return;
	}

	if (!Wire->HasNative())
	{
		RestLength = NoNative;
	}

	if (!Wire->OwnedBeginWinch.HasNative())
	{
		CurrentSpeed = NoNative;
		CurrentPulledInLength = NoNative;
		CurrentMotorForce = NoNative;
		CurrentBrakeForce = NoNative;
		return;
	}

	// We typically don't keep the entire Unreal Engine state up-to-date with the AGX Dynamics
	// objects at all time. Here, however, the user is actively looking at a particular Wire and
	// its winches so it makes sense to update them.
	//
	// To avoid the GUI having unintended side-effects on the state
	Wire->OwnedBeginWinch.ReadPropertiesFromNative();

	RestLength = FText::Format(
		LOCTEXT("RestLengthValue", "{0} cm"),
		FText::AsNumber(Wire->GetRestLength()));
	CurrentSpeed = FText::Format(
		LOCTEXT("CurrentSpeedValue", "{0} cm/s"),
		FText::AsNumber(Wire->OwnedBeginWinch.GetCurrentSpeed()));
	CurrentPulledInLength = FText::Format(
		LOCTEXT("CurrentPulledInLengthValue", "{0} cm"),
		FText::AsNumber(Wire->OwnedBeginWinch.GetPulledInLength()));
	CurrentMotorForce = FText::Format(
		LOCTEXT("CurrentMotorForceValue", "{0} N"),
		FText::AsNumber(Wire->OwnedBeginWinch.GetCurrentMotorForce()));
	CurrentBrakeForce = FText::Format(
		LOCTEXT("CurrentBrakeForceValue", "{0} N"),
		FText::AsNumber(Wire->OwnedBeginWinch.GetCurrentBrakeForce()));
}

FText FAGX_WireDetailsRuntimeBuilder::OnGetCurrentSpeed() const
{
	return CurrentSpeed;
}

FText FAGX_WireDetailsRuntimeBuilder::OnGetCurrentPulledInLength() const
{
	return CurrentPulledInLength;
}

FText FAGX_WireDetailsRuntimeBuilder::OnGetCurrentMotorForce() const
{
	return CurrentMotorForce;
}

FText FAGX_WireDetailsRuntimeBuilder::OnGetCurrentBrakeForce() const
{
	return CurrentBrakeForce;
}

#undef LOCTEXT_NAMESPACE
