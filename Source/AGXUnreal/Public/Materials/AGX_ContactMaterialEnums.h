#pragma once

#include "CoreMinimal.h"

/**
 * Specifies in what solvers the normal and friction equations will be calculated.
 */
UENUM()
enum class EAGX_ContactSolver
{
	NotDefined,

	/** Normal and friction equations calculated only in the DIRECT solver. */
	Direct,

	/** Normal and friction equations calculated only in the ITERATIVE solver. */
	Iterative,

	/**
	 * First Normal equation is calculated in the DIRECT solver, then in a second pass the normal
	 * and the friction equations are solved in the ITERATIVE solver.
	 */
	Split,

	/** Normal and friction equation calculated both in the ITERATIVE and the DIRECT solver. */
	DirectAndIterative UMETA(DisplayName = "Direct & Iterative")
};

/**
 * Specifies in what way friction is modelled.
 */
UENUM()
enum class EAGX_FrictionModel
{
	NotDefined,

	/**
	 * Box friction. Static bounds during solve. The friction box is aligned with the world axes.
	 * The normal force used by the box friction model is for new contacts estimated by the relative
	 * impact speed, mass, and gravity, or for continuous contacts equal to the the last normal
	 * force.
	 */
	BoxFriction,

	/**
	 * This model uses the current (i.e. correct) normal force received by the solver.
	 *
	 * It is computationally more expensive than the box friction model but with a more realistic
	 * dry friction.
	 */
	ScaledBoxFriction,

	/**
	 * This friction model is the default in AGX Dynamics.
	 *
	 * When this method is used with Solve type SPLIT (which is default), it will split the
	 * normal-tangential equations. I.e., the normal forces are first solved with a direct solve,
	 * and then both the normal and tangential equations are solved iteratively.
	 *
	 * For complex systems this could lead to viscous friction. This can be resolved by using
	 * 'Direct & Iterative' solve model for other constraints (hinges, etc.) involved in the system.
	 *
	 * Iterative projected cone friction model is computationally cheap. Friction forces are
	 * projected onto the friction cone, i.e., you will always get friction_force =
	 * friction_coefficient * normal_force.
	 */
	IterativeProjectedConeFriction
};
