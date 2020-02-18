#pragma once

#include "DetailLayoutBuilder.h"
#include "Containers/Array.h"
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

class AActor;

/**
* A collection of generic helper functions that can be compiled for Runtime.
*
* \see AGX_EditorUtilities.h, which is for WITH_EDITOR builds.
*
* There are also other, more specialized, AGX_.+Utilities.h collections.
*/
class FAGX_ObjectUtilities
{
public:
	/**
	* Returns array containing all child actors (and all their child actors)
	* recursively, attached to the Parent actor passed as input argument.
	* The parent itself is not included in the set.
	*/
	static void GetChildActorsOfActor(AActor* Parent, TArray<AActor*>& ChildActors);

	/**
	 * Give a list of pointer-to-base, return a new list with the elements that
	 * are of a particular derived type.
	 */
	template<typename UDerived, typename UBaseContainer>
	static TArray<UDerived*> Filter(const UBaseContainer& Collection);

	template<typename T>
	static T* FindFirstAncestorOfType(const USceneComponent& Start);

	/**
	 * Returns single object being customized from DetailBuilder if found.
	 *
	 * @param FailIfMultiple If true, nullptr is returned if multiple objects are found.
	 * If False, the first found object is returned, even if multiple objects are found.
	 */
	template <typename T>
	static T* GetSingleObjectBeingCustomized(
		IDetailLayoutBuilder& DetailBuilder, bool FailIfMultiple = true);

private:
	static void GetActorsTree(const TArray<AActor*>& CurrentLevel, TArray<AActor*>& ChildActors);
};

template <typename T>
T* FAGX_ObjectUtilities::GetSingleObjectBeingCustomized(
	IDetailLayoutBuilder& DetailBuilder, bool FailIfMultiple)
{
	static_assert(std::is_base_of<UObject, T>::value, "T must inherit from UObject");

	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);

	if (Objects.Num() == 1 || (!FailIfMultiple && Objects.Num() > 1))
		return Cast<T>(Objects[0].Get());
	else
		return nullptr;
}

template<typename T>
T* FAGX_ObjectUtilities::FindFirstAncestorOfType(const USceneComponent& Start)
{
	USceneComponent* Parent = Start.GetAttachParent();
	while (Parent != nullptr)
	{
		if (Parent->IsA<T>())
		{
			return Cast<T>(Parent);
		}
		Parent = Parent->GetAttachParent();
	}
	return nullptr;
}

template <typename UDerived, typename UBaseContainer>
TArray<UDerived*> FAGX_ObjectUtilities::Filter(const UBaseContainer& Collection)
{
	using UBase = typename std::remove_pointer<typename UBaseContainer::ElementType>::type;
	TArray<UDerived*> Result;
	for (UBase* Element : Collection)
	{
		if (UDerived* Match = Cast<UDerived>(Element))
		{
			Result.Add(Match);
		}
	}
	return Result;
}
