#pragma once

#include "CoreMinimal.h"

/** Specifies in what solvers the constraint will be solved. */
UENUM()
enum EAGX_SolveType
{
	/** Solved only in the DIRECT solver. */
	ST_DIRECT = 1 UMETA(DisplayName = "Direct"),

	/** Solved only in the ITERATIVE solver. */
	ST_ITERATIVE = (1 << 2) UMETA(DisplayName = "Iterative"),

	/** Solved both in the ITERATIVE and the DIRECT solver. */
	ST_DIRECT_AND_ITERATIVE = (ST_DIRECT | ST_ITERATIVE) UMETA(DisplayName = "Direct & Iterative")
};

/**
 * Constraint type independent index of Degree of Freedom(DOF).Does never change
 * index order layout, even in derived constraints, contrary to the AGX's native
 * constraint - specific DOF indexes.
 */
UENUM()
enum class EGenericDofIndex
{
	/** All degrees of freedom */
	ALL_DOF = -1 UMETA(DisplayName = "All"),

	/** DOF for the first translational axis */
	TRANSLATIONAL_1 = 0 UMETA(DisplayName = "Translation1"),

	/** DOF for the second translational axis */
	TRANSLATIONAL_2 = 1 UMETA(DisplayName = "Translation2"),

	/** DOF for the third translational axis */
	TRANSLATIONAL_3 = 2 UMETA(DisplayName = "Translation3"),

	/** DOF corresponding to the first rotational axis */
	ROTATIONAL_1 = 3 UMETA(DisplayName = "Rotation1"),

	/** DOF corresponding to the second rotational axis */
	ROTATIONAL_2 = 4 UMETA(DisplayName = "Rotation2"),

	/** DOF for rotation around Z-axis */
	ROTATIONAL_3 = 5 UMETA(DisplayName = "Rotation3"),
};

/**
 * Flags used to be able to identify DOFs and combine them into a bitmask.
 */
UENUM(meta = (Bitflags))
enum class EDofFlag : uint8
{
	DOF_FLAG_ALL = 0x3F UMETA(DisplayName = "All"),
	DOF_FLAG_TRANSLATIONAL_1 = 1 << 0 UMETA(DisplayName = "Translation1"),
	DOF_FLAG_TRANSLATIONAL_2 = 1 << 1 UMETA(DisplayName = "Translation2"),
	DOF_FLAG_TRANSLATIONAL_3 = 1 << 2 UMETA(DisplayName = "Translation3"),
	DOF_FLAG_ROTATIONAL_1 = 1 << 3 UMETA(DisplayName = "Rotation1"),
	DOF_FLAG_ROTATIONAL_2 = 1 << 4 UMETA(DisplayName = "Rotation2"),
	DOF_FLAG_ROTATIONAL_3 = 1 << 5 UMETA(DisplayName = "Rotation3"),
};

/**
 *
 */
UENUM()
enum class EConstraintFreeDOF : uint8
{
	FIRST UMETA(DisplayName = "First"),
	SECOND UMETA(DisplayName = "Second")
};
