// Fill out your copyright notice in the Description page of Project Settings.


#include "AGX_MeshUtilities.h"

#include "AGX_SimpleMeshComponent.h"
#include "UnrealMathUtility.h"


void AGX_MeshUtilities::MakeCube(TArray<FVector>& Positions, TArray<FVector>& Normals, TArray<uint32>& Indices, const FVector& HalfSize)
{
	// 8 Corners,
	// 6 Quads,
	// 12 Triangles,
	// 36 Indices,
	// 24 Vertices (6*4)
	// 24 Normals (6*4)

	static const TArray<FVector> StaticPositions =
	{
		FVector(-1.0, -1.0, 1.0),	FVector(1.0, -1.0, 1.0),	FVector(1.0, 1.0, 1.0),		FVector(-1.0, 1.0, 1.0),	// Up
		FVector(1.0, 1.0, 1.0),		FVector(1.0, 1.0, -1.0),	FVector(1.0, -1.0, -1.0),	FVector(1.0, -1.0, 1.0),	// Forward
		FVector(-1.0, -1.0, -1.0),	FVector(1.0, -1.0, -1.0),	FVector(1.0, 1.0, -1.0),	FVector(-1.0, 1.0, -1.0),	// Down
		FVector(-1.0, -1.0, -1.0),	FVector(-1.0, -1.0, 1.0),	FVector(-1.0, 1.0, 1.0),	FVector(-1.0, 1.0, -1.0),	// Backward
		FVector(1.0, 1.0, 1.0),		FVector(-1.0, 1.0, 1.0),	FVector(-1.0, 1.0, -1.0),	FVector(1.0, 1.0, -1.0),	// Right
		FVector(-1.0, -1.0, -1.0),	FVector(1.0, -1.0, -1.0),	FVector(1.0, -1.0, 1.0),	FVector(-1.0, -1.0, 1.0)	// Left
	};

	static const TArray<FVector> StaticNormals =
	{
		FVector(0.0, 0.0, 1.0),		FVector(0.0, 0.0, 1.0),		FVector(0.0, 0.0, 1.0),		FVector(0.0, 0.0, 1.0),		// Up
		FVector(1.0, 0.0, 0.0),		FVector(1.0, 0.0, 0.0),		FVector(1.0, 0.0, 0.0),		FVector(1.0, 0.0, 0.0),		// Forward
		FVector(0.0, 0.0, -1.0),	FVector(0.0, 0.0, -1.0),	FVector(0.0, 0.0, -1.0),	FVector(0.0, 0.0, -1.0),	// Down
		FVector(-1.0, 0.0, 0.0),	FVector(-1.0, 0.0, 0.0),	FVector(-1.0, 0.0, 0.0),	FVector(-1.0, 0.0, 0.0),	// Backward
		FVector(0.0, 1.0, 0.0),		FVector(0.0, 1.0, 0.0),		FVector(0.0, 1.0, 0.0),		FVector(0.0, 1.0, 0.0),		// Right
		FVector(0.0, -1.0, 0.0),	FVector(0.0, -1.0, 0.0),	FVector(0.0, -1.0, 0.0),	FVector(0.0, -1.0, 0.0)		// Left
	};

	static const TArray<uint32> StaticIndices =
	{
		0, 2, 1, 0, 3, 2,		// Up
		4, 5, 6, 4, 6, 7,		// Forward
		8, 9, 10, 8, 10, 11,	// Down
		12, 14, 13, 12, 15, 14,	// Backward
		16, 17, 18, 16, 18, 19,	// Right
		20, 22, 21, 20, 23, 22	// Left
	};

	Positions = StaticPositions;
	Normals = StaticNormals;
	Indices = StaticIndices;

	for (FVector &Position : Positions)
	{
		Position *= HalfSize;
	}


	check(Indices.Num() == 36);
	check(Positions.Num() == 24);
	check(Normals.Num() == 24);
}


void AGX_MeshUtilities::MakeSphere(TArray<FVector>& Positions, TArray<FVector>& Normals, TArray<uint32>& Indices, float Radius, uint32 NumSegments)
{
	if (NumSegments < 4 || Radius < 1.0e-6)
		return;

	const int32 NumStacks = NumSegments;
	const int32 NumSectors = NumStacks;
	const int32 NumVertices = (NumStacks + 1) * (NumSectors + 1);
	const int32 NumIndices = NumSectors * (6 * NumStacks - 6);

	const float SectorStep = 2.0 * PI / NumSectors;
	const float StackStep = PI / NumStacks;

	Positions.Empty(NumVertices);
	Normals.Empty(NumVertices);
	Indices.Empty(NumIndices);

	float X, Y, Z; // vertex position
	const float RadiusInv = 1.0f / Radius;
#ifdef WITH_TEXCOORDS
	float U, V; // vertex texture coordinate
#endif
	float SectorAngle;
	float StackAngle, StackRadius, StackHeight;

	// TODO: Change terminology from stack to vertical slice, and numbers accordingly!

	for (int StackIndex = 0; StackIndex <= NumStacks; ++StackIndex) // stack = vertical segment
	{
		StackAngle = PI / 2.0 - StackIndex * StackStep; // starting from pi/2 to -pi/2
		StackRadius = Radius * FMath::Cos(StackAngle);
		StackHeight = Radius * FMath::Sin(StackAngle);

		// Add (NumSectors + 1) vertices per stack. The first and last vertex
		// have same position and normal, but different tex coords.
		for (int SectorIndex = 0; SectorIndex <= NumSectors; ++SectorIndex) // sector = horizontal segment
		{
			SectorAngle = SectorIndex * SectorStep; // starting from 0 to 2pi

			X = StackRadius * FMath::Cos(SectorAngle);
			Y = StackRadius * FMath::Sin(SectorAngle);
			Z = StackHeight;

			Positions.Add(FVector(X, Y, Z));
			
			Normals.Add(FVector(
				X * RadiusInv,
				Y * RadiusInv,
				Z * RadiusInv));

#ifdef WITH_TEXCOORDS
			// vertex tex coord (u, v) range between [0, 1]
			U = (float)SectorIndex / SectorCount;
			V = (float)StackIndex / StackCount;
			TexCoords.Add(U);
			TexCoords.Add(V);
#endif
		}
	}

	// Generate index list of sphere triangles
	int K1, K2;
	for (int StackIndex = 0; StackIndex < NumStacks; ++StackIndex)
	{
		K1 = StackIndex * (NumSectors + 1); // beginning of current stack
		K2 = K1 + NumSectors + 1; // beginning of next stack

		for (int SectorIndex = 0; SectorIndex < NumSectors; ++SectorIndex, ++K1, ++K2)
		{
			// 2 triangles per sector excluding first and last stacks
			// K1 => K2 => K1+1
			if (StackIndex != 0)
			{
				Indices.Add(K1);
				Indices.Add(K1 + 1);
				Indices.Add(K2);
			}

			// K1+1 => K2 => K2+1
			if (StackIndex != (NumStacks - 1))
			{
				Indices.Add(K1 + 1);
				Indices.Add(K2 + 1);
				Indices.Add(K2);
			}
		}
	}

	check(Indices.Num() == NumIndices);
	check(Positions.Num() == NumVertices);
	check(Normals.Num() == NumVertices);
}


void AGX_MeshUtilities::MakeCylinder(TArray<FVector>& Positions, TArray<FVector>& Normals, TArray<uint32>& Indices, float Radius, float Height, uint32 NumCircleSegments, uint32 NumHeightSegments)
{
	if (NumCircleSegments < 4 || NumCircleSegments > uint32(TNumericLimits<uint16>::Max()) ||
		NumHeightSegments < 1 || NumHeightSegments > uint32(TNumericLimits<uint16>::Max()) || 
		Radius < 1.0e-6 || Height < 1.0e-6)
		return;

	const int32 NumVertexRows = NumHeightSegments + 1;
	const int32 NumVertexColumns = NumCircleSegments + 1;
	const int32 NumCaps = 2;
	const int32 NumVertexRowsAndCaps = NumVertexRows + 2;
	const int32 NumVertices = NumVertexRowsAndCaps * NumVertexColumns;
	const int32 NumIndices = NumHeightSegments * NumCircleSegments * 6 + NumCaps * (NumCircleSegments - 2) * 3;

	const float SegmentSize = 2.0 * PI / NumCircleSegments;
	const float RadiusInv = 1.0f / Radius;

	Positions.Empty(NumVertices);
	Normals.Empty(NumVertices);
	Indices.Empty(NumIndices);

	float X, Y, Z; // vertex position
#ifdef WITH_TEXCOORDS
	float U, V; // vertex texture coordinate
#endif

	// Generate vertex attributes.

	// Add vertices per horizontal row on the cylinder, with bottom and top vertex rows
	// duplicated for the caps, because they need different normals and tex coords.
	// The sequence of vertices are as follow:
	// Bottom Cap, Bottom Row, Bottom Row + 1, ..., Top Row - 1, Top Row, Top Cap
	for (int CapsAndRowIndex = 0; CapsAndRowIndex < NumVertexRowsAndCaps; ++CapsAndRowIndex)
	{
		const int CapIndex = CapsAndRowIndex == 0 ? 0 : (CapsAndRowIndex == NumVertexRows + 1 ? 1 : -1);
		const bool IsCap = CapIndex != -1;
		const int RowIndex = FMath::Clamp<int>(CapsAndRowIndex - 1, 0, NumVertexRows - 1);
		const float RowHeight = Height * RowIndex / (NumVertexRows - 1) - Height * 0.5f;
		
		// Add NumVertexColumns vertices in a circle per vertex row. The first and last
		// vertex in the same row have same position and normal, but different tex coords.
		for (int ColumnIndex = 0; ColumnIndex < NumVertexColumns; ++ColumnIndex)
		{
			float ColumnAngle = ColumnIndex * SegmentSize;

			X = Radius * FMath::Cos(ColumnAngle);
			Y = RowHeight;
			Z = Radius * FMath::Sin(ColumnAngle);

			Positions.Add(FVector(X, Y, Z));

			switch (CapIndex)
			{
			case 0: // bottom cap
				Normals.Add(FVector(0.0f, -1.0f, 0.0f)); break;
			case 1: // top cap
				Normals.Add(FVector(0.0f, 1.0f, 0.0f)); break;
			default: // normal row
				Normals.Add(FVector(X * RadiusInv, 0.0f, Z * RadiusInv)); break;
			}

#ifdef WITH_TEXCOORDS
			// vertex tex coord (u, v) range between [0, 1]
			U = IsCap ? X : ((float)ColumnIndex / NumCircleSegments);
			V = IsCap ? Z : ((float)RowIndex / (NumVertexRows - 1));
			TexCoords.Add(U);
			TexCoords.Add(V);
#endif
		}
	}

	// Generate triangle indexes for the side segments of the capsule.
	int K0, K1;
	for (int HeightSegmentIndex = 0; HeightSegmentIndex < int32(NumHeightSegments); ++HeightSegmentIndex)
	{
		K0 = NumVertexColumns + HeightSegmentIndex * NumVertexColumns; // first vertex in bottom vertex row of height segment (offset by cap)
		K1 = K0 + NumVertexColumns; // first vertex in next row

		for (int CircleSegmentIndex = 0; CircleSegmentIndex < int32(NumCircleSegments); ++CircleSegmentIndex, ++K0, ++K1)
		{
			// 2 triangles per segment (i.e. quad)

			// K0 => K0+1 => K1
			Indices.Add(K0);
			Indices.Add(K0 + 1);
			Indices.Add(K1);

			// K0+1 => K1+1 => K1
			Indices.Add(K0 + 1);
			Indices.Add(K1 + 1);
			Indices.Add(K1);
		}
	}

	// Generate triangle indexes for the caps, with triangle fan pattern.
	int K2;
	for (int CapIndex = 0; CapIndex < NumCaps; ++CapIndex)
	{
		K0 = CapIndex * (NumVertexRowsAndCaps - 1) * NumVertexColumns; // first vertex in cap row
		K1 = K0 + 1; // second vertex in cap row
		K2 = K1 + 1; // third vertex in cap row

		for (int CircleSegmentIndex = 1; CircleSegmentIndex < int32(NumCircleSegments) - 1; ++CircleSegmentIndex, ++K1, ++K2)
		{
			// 1 triangles per segment (except for first and last segment)

			if (CapIndex == 0)
			{
				// K2 => K1 => K0
				Indices.Add(K2);
				Indices.Add(K1);
				Indices.Add(K0);
			}
			else
			{
				// K0 => K1 => K2
				Indices.Add(K0);
				Indices.Add(K1);
				Indices.Add(K2);
			}
		}
	}

	check(Indices.Num() == NumIndices);
	check(Positions.Num() == NumVertices);
	check(Normals.Num() == NumVertices);
}
