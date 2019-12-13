// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class FDynamicMeshIndexBuffer32;
struct FStaticMeshVertexBuffers;
struct FAGX_SimpleMeshTriangle;

/// \todo Each nested ***ConstructionData classes below could contain the respective Make-function
/// as a member function, to even furter reduce potential usage mistakes!

/**
 * Provides helper functions for creating custom Unreal Meshes.
 */
class AGXUNREAL_API AGX_MeshUtilities
{
public:
	static void MakeCube(
		TArray<FVector>& Positions, TArray<FVector>& Normals, TArray<uint32>& Indices,
		const FVector& HalfSize);

	/**
	 * Used to define the geometry of a mesh sphere, and also to know the number of vertices and
	 * indices in advance.
	 */
	struct SphereConstructionData
	{
		// Input:
		const float Radius;
		const float Segments;

		// Derived:
		const uint32 Stacks;
		const uint32 Sectors;
		const uint32 Vertices;
		const uint32 Indices;

		SphereConstructionData(float InRadius, uint32 InNumSegments);

		void AppendBufferSizes(uint32& InOutNumVertices, uint32& InOutNumIndices) const;
	};

	/// \todo Change to use SphereConstructionData as input.
	static void MakeSphere(
		TArray<FVector>& Positions, TArray<FVector>& Normals, TArray<uint32>& Indices, float Radius,
		uint32 NumSegments);

	/**
	 * Appends buffers with geometry data for a sphere, centered at origin.
	 *
	 * Buffers will not be resized, and must therefore already have enough space to contain the data
	 * to be written. Use CylinderConstructionData.AppendBufferSizes to calculate how much data to
	 * allocate in advance.
	 *
	 * Will start writing from NextFreeVertex and NextFreeIndex, and update them before returning
	 * such that they point to one past the last added vertex and index.
	 */
	static void MakeSphere(
		FStaticMeshVertexBuffers& VertexBuffers, FDynamicMeshIndexBuffer32& IndexBuffer,
		uint32& NextFreeVertex, uint32& NextFreeIndex,
		const SphereConstructionData& ConstructionData);

	/**
	 * Used to define the geometry of a mesh cylinder, and also to know the number of vertices and
	 * indices in advance.
	 */
	struct CylinderConstructionData
	{
		// Input:
		const float Radius;
		const float Height;
		const uint32 CircleSegments;
		const uint32 HeightSegments;
		const FLinearColor& MiddleColor;
		const FLinearColor& OuterColor;

		// Derived:
		const uint32 VertexRows;
		const uint32 VertexColumns;
		const uint32 Caps;
		const uint32 VertexRowsAndCaps;
		const uint32 Vertices;
		const uint32 Indices;

		CylinderConstructionData(
			float InRadius, float InHeight, uint32 InNumCircleSegments, uint32 InNumHeightSegments,
			const FLinearColor& InMiddleColor = FLinearColor(1, 1, 1, 1),
			const FLinearColor& InOuterColor = FLinearColor(1, 1, 1, 1));

		void AppendBufferSizes(uint32& InOutNumVertices, uint32& InOutNumIndices) const;
	};

	/**
	 * Initializes buffers with geometry data for a cylinder extending uniformly along the Y-Axis,
	 * centered at origin.
	 */
	static void MakeCylinder(
		TArray<FVector>& Positions, TArray<FVector>& Normals, TArray<uint32>& Indices,
		const CylinderConstructionData& ConstructionData);

	/**
	 * Appends buffers with geometry data for a cylinder extending uniformly along the Z-Axis,
	 * centered at origin.
	 *
	 * Buffers will not be resized, and must therefore already have enough space to contain the data
	 * to be written. Use CylinderConstructionData.AppendBufferSizes to calculate how much data to
	 * allocate in advance.
	 *
	 * Will start writing from NextFreeVertex and NextFreeIndex, and update them before returning
	 * such that they point to one past the last added vertex and index.
	 */
	static void MakeCylinder(
		FStaticMeshVertexBuffers& VertexBuffers, FDynamicMeshIndexBuffer32& IndexBuffer,
		uint32& NextFreeVertex, uint32& NextFreeIndex,
		const CylinderConstructionData& ConstructionData);

	/**
	 * Used to define the geometry of a mesh arrow, and also to know the number of vertices and
	 * indices in advance.
	 */
	struct CylindricalArrowConstructionData
	{
		// Input:
		const float CylinderRadius;
		const float CylinderHeight;
		const float ConeRadius;
		const float ConeHeight;
		const bool bBottomCap;
		const uint32 CircleSegments;
		const FLinearColor BaseColor;
		const FLinearColor TopColor;

		// Derived:
		const uint32 VertexRows;
		const uint32 VertexColumns;
		const uint32 Vertices;
		const uint32 Indices;

		CylindricalArrowConstructionData(
			float InCylinderRadius, float InCylinderHeight, float InConeRadius, float InConeHeight,
			bool bInBottomCap, uint32 InNumCircleSegments, const FLinearColor& InBaseColor,
			const FLinearColor& InTopColor);

		void AppendBufferSizes(uint32& InOutNumVertices, uint32& InOutNumIndices) const;
	};

	/**
	 * Appends buffers with geometry data for a cylinder extending uniformly along the Z-Axis,
	 * centered at origin.
	 *
	 * Buffers will not be resized, and must therefore already have enough space to contain the data
	 * to be written. Use CylindricalArrowConstructionData.AppendBufferSizes to calculate how much
	 * data to allocate in advance.
	 *
	 * Will start writing from NextFreeVertex and NextFreeIndex, and update them before returning
	 * such that they point to one past the last added vertex and index.
	 */
	static void MakeCylindricalArrow(
		FStaticMeshVertexBuffers& VertexBuffers, FDynamicMeshIndexBuffer32& IndexBuffer,
		uint32& NextFreeVertex, uint32& NextFreeIndex,
		const CylindricalArrowConstructionData& ConstructionData);

	/**
	 * Used to define the geometry of a flat bendable arrow, and also to know the number of vertices
	 * and indices in advance.
	 */
	struct BendableArrowConstructionData
	{
		// Input:
		const float RectangleWidth;
		const float RectangleLength;
		const float TriangleWidth;
		const float TriangleLength;
		const float BendAngle; // Radians of a circle the arrow will bend along. Zero means no bend.
		const uint32 Segments;
		const FLinearColor BaseColor;
		const FLinearColor TopColor;

		// Derived:
		const uint32 RectangleSegments;
		const uint32 TriangleSegments;
		const uint32 RectangleVertexRows;
		const uint32 TriangleVertexRows;
		const uint32 Vertices;
		const uint32 Indices;

		BendableArrowConstructionData(
			float InRectangleWidth, float InRectangleLength, float InTriangleWidth,
			float InTriangleLength, float InBendAngle, uint32 InNumSegments,
			const FLinearColor& InBaseColor, const FLinearColor& InTopColor);

		void AppendBufferSizes(uint32& InOutNumVertices, uint32& InOutNumIndices) const;
	};

	/**
	 * Appends buffers with geometry data for a flat bendable arrow, extending initially along the
	 * Z-Axis, centered at origin, and bending counter clockwise arond the Y-Axis.
	 *
	 * Buffers will not be resized, and must therefore already have enough space to contain the data
	 * to be written. Use CylindricalArrowConstructionData.AppendBufferSizes to calculate how much
	 * data to allocate in advance.
	 *
	 * Will start writing from NextFreeVertex and NextFreeIndex, and update them before returning
	 * such that they point to one past the last added vertex and index.
	 */
	static void MakeBendableArrow(
		FStaticMeshVertexBuffers& VertexBuffers, FDynamicMeshIndexBuffer32& IndexBuffer,
		uint32& NextFreeVertex, uint32& NextFreeIndex,
		const BendableArrowConstructionData& ConstructionData);

	static void PrintMeshToLog(
		const FStaticMeshVertexBuffers& VertexBuffers,
		const FDynamicMeshIndexBuffer32& IndexBuffer);

	/**
	 * Used to define the geometry of a mesh cylinder, and also to know the number of vertices and
	 * indices in advance.
	 */
	struct DiskArrayConstructionData
	{
		// Input:
		const float Radius;
		const uint32 CircleSegments;
		const float Spacing;
		const uint32 Disks;
		const bool bTwoSided;
		const FLinearColor MiddleDiskColor;
		const FLinearColor OuterDiskColor;
		TArray<FTransform> SpacingsOverride; // optional, Spacing will be ignored if defined. Zero
											 // or num Disks items.

		// Derived:
		const uint32 SidesPerDisk;
		const uint32 VerticesPerSide;
		const uint32 VerticesPerDisk;
		const uint32 Vertices;
		const uint32 Indices;

		DiskArrayConstructionData(
			float InRadius, uint32 InNumCircleSegments, float InSpacing, uint32 InDisks,
			bool bInTwoSided, const FLinearColor InMiddleDiskColor,
			const FLinearColor InOuterDiskColor, TArray<FTransform> InSpacingsOverride = {});

		void AppendBufferSizes(uint32& InOutNumVertices, uint32& InOutNumIndices) const;
	};

	static void MakeDiskArray(
		FStaticMeshVertexBuffers& VertexBuffers, FDynamicMeshIndexBuffer32& IndexBuffer,
		uint32& NextFreeVertex, uint32& NextFreeIndex, const DiskArrayConstructionData& Data);
};
