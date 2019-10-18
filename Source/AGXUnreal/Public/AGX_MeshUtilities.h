// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

struct FAGX_SimpleMeshTriangle;


/**
 * Provides helper functions for creating custom Unreal Meshes.
 */
class AGXUNREAL_API AGX_MeshUtilities
{
public:

	static void MakeCube(TArray<FAGX_SimpleMeshTriangle>& Triangles, FVector HalfSize);

	static void MakeSphere(TArray<FAGX_SimpleMeshTriangle>& Triangles, float Radius, uint32 NumSegments);

};
