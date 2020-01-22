#pragma once

#include "CoreMinimal.h"

class IDetailLayoutBuilder;

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
