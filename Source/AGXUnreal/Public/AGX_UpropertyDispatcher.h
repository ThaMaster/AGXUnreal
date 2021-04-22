#pragma once

#if WITH_EDITOR

// Unreal Engine includes.
#include "Containers/Map.h"
#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformMisc.h"

/**
 * We identify properties by a (Member, Property) pair. Each such pair is associated with a
 * callback. "Member" is the name of the member on the UObject, the UObject on which
 * PostEditChangeProperty is called. "Property" is the first nesting of the member. This is used
 * when the member is a struct containing many properties and we only wish to update one at the
 * time.
 *
 * For properties directly on the UObject Member and Property will be the same.
 *
 * Not that for nesting deeper than two levels these doesn't directly correspond to the Member and
 * Property parameters passed to PostEditPropertyChanged. They are the first and last properties of
 * a nesting chain while here we store the first and second properties.
 *
 * For examples, consider Constraint1DofComponent > RangeController > Range > Min. Here Member would
 * be "RangeController" and Property would be "Range". The same callback would be called for Min and
 * Max, the other member of Range.
 *
 * For immediate member structs that you want to handle with a single callback register a callback
 * on the Member name only and do not register any callback for the nested properties. This is
 * useful for Members that are e.g. a FVector where the (Member, Property) pair would be e.g.
 * (Velocity, X) but we might want to  update the entire velocity as a single chunk. Then add a
 * callback for just "Velocity" and not ("Velocity", "X") etc.
 */
struct FAGX_NamePair
{
	// The name of the root property on the modified instance. This can be a struct or other
	// aggregate type, but it can also be the actual leaf property if the property is a primitive
	// type. If it is the leaf property then Member == Property.
	FName Member;

	// The name of the property that was changed. This can be either equal to Member (for primitive
	// properties), a direct member of Member (for simple struct properties) or the name of a struct
	// (for nested structs).
	FName Property;

	// The Key should be read as:
	//   The property 'Member.Property' was changed, but there can be additional levels of nesting.
};

inline bool operator==(const FAGX_NamePair& LHS, const FAGX_NamePair& RHS)
{
	return LHS.Member == RHS.Member && LHS.Property == RHS.Property;
}

inline uint32 GetTypeHash(const FAGX_NamePair& Pair)
{
	const uint32 Member = GetTypeHash(Pair.Member);
	const uint32 Property = GetTypeHash(Pair.Property);
	const uint32 Hash = HashCombine(Member, Property);
	return Hash;
}

/**
 * The UpropertyDispatcher is a collection of (Property, Callback) pairs. A UObject subclass that
 * need to handle many members in its PostEditChangeProperty or PostEditChangeChainProperty can
 * delegate the name testing to this class and have a callback called for each time a property is
 * edited.
 *
 * @tparam T The type of the object holding this UpropertyDispatcher. A pointer of this type will be
 * passed to each callback.
 */
template <typename T>
struct AGXUNREAL_API FAGX_UpropertyDispatcher
{
public:
	using UpdatePropertyFunction = TFunction<void(T*)>;

	static FAGX_UpropertyDispatcher<T>& Get();

	/**
	 * Add a callback for a direct member property. The case where Member and Property have the same
	 * value.
	 * @param MemberAndProperty The name of the property.
	 * @param Function The function to call when the property is changed.
	 */
	void Add(const FName& MemberAndProperty, UpdatePropertyFunction&& Function);

	/**
	 * Add a callback for a struct member property. Member should be the name of the struct that is
	 * the outermost member and Property should be the name of the property within that struct.
	 * @param Member The top-most member.
	 * @param Property The member within the top-most member.
	 * @param Function The function to call when the property is changed.
	 */
	void Add(const FName& Member, const FName& Property, UpdatePropertyFunction&& Function);

	/**
	 * Run the function associated with the property.
	 * @param Member The name of the direct member that was changed.
	 * @param Property The name of the nested property that was changed.
	 * @param Object The object on which the member was changed. Is passed to the callback.
	 * @return True if a function was run, false otherwise.
	 */
	bool Trigger(const FName& Member, const FName& Property, T* Object);

	bool IsInitialized() const;

	UpdatePropertyFunction* GetFunction(const FName& Member, const FName& Property);

private:
	TMap<FAGX_NamePair, UpdatePropertyFunction> Functions;
};

template <typename T>
FAGX_UpropertyDispatcher<T>& FAGX_UpropertyDispatcher<T>::Get()
{
	static FAGX_UpropertyDispatcher<T> Instance;
	return Instance;
}

template <typename T>
void FAGX_UpropertyDispatcher<T>::Add(
	const FName& MemberAndProperty, UpdatePropertyFunction&& Function)
{
	Functions.Add(FAGX_NamePair {MemberAndProperty, MemberAndProperty}, std::move(Function));
}

template <typename T>
void FAGX_UpropertyDispatcher<T>::Add(
	const FName& Member, const FName& Property, UpdatePropertyFunction&& Function)
{
	Functions.Add(FAGX_NamePair {Member, Property}, std::move(Function));
}

template <typename T>
bool FAGX_UpropertyDispatcher<T>::Trigger(const FName& Member, const FName& Property, T* Object)
{
	UpdatePropertyFunction* Function = GetFunction(Member, Property);
	if (Function == nullptr)
	{
		return false;
	}

	(*Function)(Object);
	return true;
}

template <typename T>
bool FAGX_UpropertyDispatcher<T>::IsInitialized() const
{
	return Functions.Num() != 0;
}

template <typename T>
typename FAGX_UpropertyDispatcher<T>::UpdatePropertyFunction*
FAGX_UpropertyDispatcher<T>::GetFunction(const FName& Member, const FName& Property)
{
	// First see if we  have a callback for this specific property.
	UpdatePropertyFunction* Function = Functions.Find({Member, Property});
	if (Function != nullptr)
	{
		return Function;
	}

	// Did not have a callback for the specific property, see if we have a callback for the entire
	// member.
	Function = Functions.Find({Member, Member});
	if (Function != nullptr)
	{
		return Function;
	}

	return nullptr;
}

#endif
