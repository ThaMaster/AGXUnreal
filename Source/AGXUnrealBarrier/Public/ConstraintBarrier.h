#pragma once

#include <memory>

struct FConstraintRef;
class FRigidBodyBarrier;

/**
 * Acts as an interface to a native AGX constraint, and encapsulates it so that
 * it is completely hidden from code that includes this file.
 *
 * To support specialized native constraint types, a barrier class deriving from
 * this class has to be created for each constraint type. The derived class
 * does only need to implement AllocateNativeImpl (see function comment), and
 * supply an interface specific for the specialization. It does not need to hold
 * a reference to the native object. Because the derived class creates the native
 * constraint object in AllocateNativeImpl, it can also safely cast NativeRef->Native
 * to that same type whenever necessary for temporary usage in implementation of
 * its specialized interface.
 */
class AGXUNREALBARRIER_API FConstraintBarrier
{
public:
	FConstraintBarrier();
	virtual ~FConstraintBarrier();

	bool HasNative() const;
	FConstraintRef* GetNative();
	const FConstraintRef* GetNative() const;

	void AllocateNative(const FRigidBodyBarrier *Rb1, const FRigidBodyBarrier *Rb2);
	void ReleaseNative();

	void SetCompliance(double Compliance);
	double GetCompliance() const;

	void SetDamping(double Damping);
	double GetDamping() const;

private:
	FConstraintBarrier(const FConstraintBarrier&) = delete;
	void operator=(const FConstraintBarrier&) = delete;

private:

	/**
	 * Called from AllocateNative. Each subclass msut override this function and within
	 * it create the correct agx::Constraint object, and assign it to NativeRef->Native.
	 *
	 * The override should not call the override of the parent class, to avoid multiple
	 * objects being created in a deeper inheritance tree! 
	 *
	 * The derived class should not store a reference to the native object!
	 */
	virtual void AllocateNativeImpl(const FRigidBodyBarrier *Rb1, const FRigidBodyBarrier *Rb2) = 0;
	//virtual void ReleaseNativeImpl() = 0;

protected:
	std::unique_ptr<FConstraintRef> NativeRef;
	// NativeRef is created by the lowermost subclass, and NativeRef->Native should be
	// type-casted whenever a subclass needs the derived interface (e.g. to agx::LockJoint).
};
