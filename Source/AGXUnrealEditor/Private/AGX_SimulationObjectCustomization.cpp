#include "AGX_SimulationObjectCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_EditorUtilities.h"
#include "AGX_SimulationObjectComponent.h"

// Unreal Engine includes.
#include "Components/StaticMeshComponent.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Engine/StaticMesh.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/AggregateGeom.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSpinBox.h"

#define LOCTEXT_NAMESPACE "FAGX_SimulationObjectCustomization"

TSharedRef<IDetailCustomization> FAGX_SimulationObjectCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_SimulationObjectCustomization);
}

namespace FAGX_SimulationObjectCustomization_helpers
{
	class SShapeWidget : public SCompoundWidget
	{
	public:
		SLATE_BEGIN_ARGS(SShapeWidget)
			: _Shape(nullptr)
			, _ShapeType(EAggCollisionShape::Unknown)
		{
		}

		SLATE_ARGUMENT(FAGX_Shape*, Shape)
		SLATE_ARGUMENT(EAggCollisionShape::Type, ShapeType)

		SLATE_END_ARGS()

		void Construct(const FArguments& InArguments);
	};

	void SShapeWidget::Construct(const FArguments& InArguments)
	{
		// clang-format off
		FAGX_Shape* Shape = InArguments._Shape;
		EAggCollisionShape::Type ShapeType = InArguments._ShapeType;
		if (Shape == nullptr)
		{
			UE_LOG(LogAGX, Error, TEXT("Got nullptr shape in SShapeWidget. That's unexpected"));
			ChildSlot
			[
				SNew(STextBlock)
				.ColorAndOpacity(FLinearColor::Red)
				.Text(LOCTEXT("SShapeWidget_ShapeNull", "Get nullptr shape. That's unexpected"))
			];
			return;
		}
		if (ShapeType == EAggCollisionShape::Unknown)
		{
			UE_LOG(LogAGX, Error, TEXT("Got unknown shape type in SShapeWidget. That's unexpected"));
			ChildSlot
			[
				SNew(STextBlock)
				.ColorAndOpacity(FLinearColor::Red)
				.Text(LOCTEXT(
					"SShapeWidget_ShapeTypeUnknown",
					"Got unknown shape type. That's unexpected"))
			];
			return;
		}
		ChildSlot
		[
			SNew(SBorder)
			.BorderBackgroundColor(FLinearColor(1.0f, 0.2f, 0.0f))
			.Padding(FMargin(5.0f, 5.0f))
			.Content()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Shape ", "I'm a SShapeWidget"))
				]
			]
		];
		// clang-format on
	}
}

void FAGX_SimulationObjectCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	using namespace FAGX_SimulationObjectCustomization_helpers;

	UE_LOG(LogAGX, Warning, TEXT("SimulationObject customization running"));

	UAGX_SimulationObjectComponent* SimulationObject =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_SimulationObjectComponent>(
			DetailBuilder);
	if (!SimulationObject)
	{
		// We're modifying multiple (or zero) Simulation objects. Either handle that in the below
		// code or simply fall back to the default Details Panel view.
		//
		// The latter is probably unexpected for the user, but kind of works for now.

		/// \todo The intent is to fall back to the default Details Panel.
		/// Is this enough to achieve that?
		return;
	}

	IDetailCategoryBuilder& AgxCategory =
		DetailBuilder.EditCategory("AGX Dynamics", FText::GetEmpty(), ECategoryPriority::Important);

	/// \todo Create an orange border around AgxCategory.





	// clang-format off

#if 0
	// Create button to add a new sphere.
	// Not used anymore now that we read collision shapes from the Static Mesh.
	// Can we modify the StaticMesh from here?
	// Would that edit the source Asset or this SimulationObject's instance?
	AgxCategory.AddCustomRow(FText::FromString("Add sphere"))
	[
		SNew(SButton)
		.Text(LOCTEXT("AddSphere", "Add sphere"))
		.ToolTipText(LOCTEXT("AddSphereTooltip", "Add a sphere to this simulation object"))
		.OnClicked_Lambda([SimulationObject]() {
			UE_LOG(LogAGX, Warning, TEXT("Adding a sphere."));
			SimulationObject->Spheres.Add({1.0});
			/// \todo We may want to re-run CustomizeDetails after this change.
			/// How do we trigger that?
			return FReply::Handled();
		})
	];
#endif

	AgxCategory.AddCustomRow(FText::FromString(""))
	[
		SNew(SButton)
		.Text(LOCTEXT("RefreshCollisionShapes", "Refresh collision shapes"))
		.ToolTipText(LOCTEXT(
			"RefreshCollisionShapesTip",
			"Fetch the current collision shapes from the static mesh and populate the physics shape "
			"lists."))
		.OnClicked_Lambda([SimulationObject]() {
			SimulationObject->RefreshCollisionShapes();
			return FReply::Handled();
		})
	];

	SimulationObject->RefreshCollisionShapes();

	if (SimulationObject->Spheres.Num() > 0)
	{
		AgxCategory.AddCustomRow(FText::FromString("ShapeWidget"))
		[
			SNew(SShapeWidget)
			.Shape(nullptr)
			.ShapeType(EAggCollisionShape::Sphere)
		];
		AgxCategory.AddCustomRow(FText::FromString("ShapeWidget"))
		[
				SNew(SShapeWidget)
				.Shape(&SimulationObject->Spheres[0])
				.ShapeType(EAggCollisionShape::Unknown)
		];
		AgxCategory.AddCustomRow(FText::FromString("ShapeWidget"))
		[
			SNew(SShapeWidget)
			.Shape(&SimulationObject->Spheres[0])
			.ShapeType(EAggCollisionShape::Sphere)
		];
	}


#if 0
	UStaticMesh* Mesh = SimulationObject->GetStaticMesh();
	FKAggregateGeom& Shapes = Mesh->BodySetup->AggGeom;

	for (FKSphereElem& Sphere : Shapes.SphereElems)
	{

	}

	Shapes.BoxElems;
#endif


#if 1
	IDetailCategoryBuilder& Spheres = AgxCategory;
#else
	IDetailCategoryBuilder&  Spheres = DetailBuilder.EditCategory(
		"AGX Dynamics|Spheres", LOCTEXT("CatSpheresName", "Spheres"));
#endif

	AgxCategory.AddCustomRow(FText::FromString("SphereTitle"))
	[
		SNew(STextBlock)
		.Text(LOCTEXT("BoxAndSlotSlot", "Using box and AddSlot:"))
	];

	FDetailWidgetRow& SpheresRow = Spheres.AddCustomRow(FText::FromString("Spheres"));
	TSharedRef<SBorder> SpheresBorder =
		SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(1.0f, 0.2f, 0.0f))
		.Padding(FMargin(5.0f, 5.0f));

	TSharedRef<SVerticalBox> SpheresBox = SNew(SVerticalBox);

	for (auto& Sphere : SimulationObject->Spheres)
	{
		SpheresBox->AddSlot()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SphereRadius", "Sphere radius in box"))
		];
	}

	SpheresBorder->SetContent(SpheresBox);
	SpheresRow
	[
		SpheresBorder
	];


	AgxCategory.AddCustomRow(FText::FromString("SphereTitle"))
	[
			SNew(STextBlock)
			.Text(LOCTEXT("BorderAndTextBlock", "Using border and text block:"))
	];


	for (auto& Sphere : SimulationObject->Spheres)
	{
	#if 1
		Spheres.AddCustomRow(FText::FromString("Spheres"))
		[
			SNew(SBorder)
			.BorderBackgroundColor(FLinearColor(1.0f, 0.2f, 0.0f))
			.Padding(FMargin(5, 5))
			.Content()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SphereRadius", "SphereRadius"))
			]
		];
	#endif
	#if 0
		Spheres.AddCustomRow(FText::FromString("Spheres"))
		.WholeRowContent()
		[
			// Q: This border is not rendered. Why not?
			// A: I don't think we're supposed to use both `WholeRowContent`
			// and the Name/ValueContent slots on the same widget. Not sure tho.
			SNew(SBorder)
			.ColorAndOpacity(FLinearColor::Blue)
		]
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SphereRadius", "Sphere radius"))
		]
		.ValueContent()
		[
			SNew(SSpinBox<float>)
			.MinValue(0.0f)
			.MaxValue(10.0f)
		];
	#endif
	#if 0
		AgxCategory.AddCustomRow(FText::FText::FromString("Spheres"))
		[
			SNew()
		];
	#endif
	}

	AgxCategory.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("TestButton", "Test button."))
	 		.ToolTipText(LOCTEXT("TestButtonTooltip", "Test button tooltip."))
			.OnClicked_Lambda([]() {
				UE_LOG(LogAGX, Warning, TEXT("Button clicked."));
				return FReply::Handled();
			})
		]
	];

	// clang-format on
}

#undef LOCTEXT_NAMESPACE



#if 0
A question:

In `CustomizeDetails`, is it possible to add a property widget nested somewhere inside a row created with `AddCustomRow`, inside a `SVerticalBox` for example?


Category.AddCustomRow(LOCTEXT("FilterKey", "FilterValue"))
.WholeRowContent()
[
    SNew(SVerticalBox)
    + SHorizontalBox::Slot()
    .AutoWidth()
    [
        // AddProperty doesn't work because it doesn't return a SWidget, but I
		// can't find any Property Widget creator function that does.
        Category.CreateProperty(DetailBuilder.GetProperty("MyProperty"))
    ]
    + // More slots here.
];
#endif
