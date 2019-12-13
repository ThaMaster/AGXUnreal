// Fill out your copyright notice in the Description page of Project Settings.

#include "Utilities/AGX_MeshUtilities.h"

#include <limits>
#include <algorithm>

#include "UnrealMathUtility.h"

#include "AGX_SimpleMeshComponent.h"
#include "AGX_LogCategory.h"

#define CONE_SINGULARITY

void AGX_MeshUtilities::MakeCube(
	TArray<FVector>& Positions, TArray<FVector>& Normals, TArray<uint32>& Indices,
	const FVector& HalfSize)
{
	// 8 Corners,
	// 6 Quads,
	// 12 Triangles,
	// 36 Indices,
	// 24 Vertices (6*4)
	// 24 Normals (6*4)

	// clang-format off
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
	// clang-format on

	Positions = StaticPositions;
	Normals = StaticNormals;
	Indices = StaticIndices;

	for (FVector& Position : Positions)
	{
		Position *= HalfSize;
	}

	check(Indices.Num() == 36);
	check(Positions.Num() == 24);
	check(Normals.Num() == 24);
}

AGX_MeshUtilities::SphereConstructionData::SphereConstructionData(
	float InRadius, uint32 InNumSegments)
	: Radius(InRadius)
	, Segments(InNumSegments)
	, Stacks(InNumSegments)
	, Sectors(InNumSegments)
	, Vertices((Stacks + 1) * (Sectors + 1))
	, Indices(Sectors * (6 * Stacks - 6))
{
}

void AGX_MeshUtilities::SphereConstructionData::AppendBufferSizes(
	uint32& InOutNumVertices, uint32& InOutNumIndices) const
{
	InOutNumVertices += Vertices;
	InOutNumIndices += Indices;
}

void AGX_MeshUtilities::MakeSphere(
	TArray<FVector>& Positions, TArray<FVector>& Normals, TArray<uint32>& Indices, float Radius,
	uint32 NumSegments)
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
		for (int SectorIndex = 0; SectorIndex <= NumSectors;
			 ++SectorIndex) // sector = horizontal segment
		{
			SectorAngle = SectorIndex * SectorStep; // starting from 0 to 2pi

			X = StackRadius * FMath::Cos(SectorAngle);
			Y = StackRadius * FMath::Sin(SectorAngle);
			Z = StackHeight;

			Positions.Add(FVector(X, Y, Z));

			Normals.Add(FVector(X * RadiusInv, Y * RadiusInv, Z * RadiusInv));

#ifdef WITH_TEXCOORDS
			// vertex tex coord (u, v) range between [0, 1]
			U = (float) SectorIndex / SectorCount;
			V = (float) StackIndex / StackCount;
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

void AGX_MeshUtilities::MakeSphere(
	FStaticMeshVertexBuffers& VertexBuffers, FDynamicMeshIndexBuffer32& IndexBuffer,
	uint32& NextFreeVertex, uint32& NextFreeIndex, const SphereConstructionData& Data)
{
	check(Data.Segments >= 4);
	check(Data.Segments <= uint32(TNumericLimits<uint16>::Max()));
	check(Data.Radius >= 1.0e-6);

	check(NextFreeVertex + Data.Vertices <= VertexBuffers.PositionVertexBuffer.GetNumVertices());
	check(NextFreeVertex + Data.Vertices <= VertexBuffers.StaticMeshVertexBuffer.GetNumVertices());
	check(NextFreeVertex + Data.Vertices <= VertexBuffers.ColorVertexBuffer.GetNumVertices());
	check(NextFreeIndex + Data.Indices <= static_cast<uint32>(IndexBuffer.Indices.Num()));

	const uint32 FirstVertex = NextFreeVertex;
	const uint32 FirstIndex = NextFreeIndex;

	const float SectorStep = 2.0 * PI / Data.Sectors;
	const float StackStep = PI / Data.Stacks;
	const float RadiusInv = 1.0f / Data.Radius;

	FVector Position, TangentX, TangentY, TangentZ;
	FColor Color = FColor::White; /// \todo Make configurable!
	FVector2D TexCoord;

	float SectorAngle;
	float StackAngle, StackRadius, StackHeight;

	// TODO: Change terminology from stack to vertical slice, and numbers accordingly!

	for (uint32 StackIndex = 0; StackIndex <= Data.Stacks; ++StackIndex) // stack = vertical segment
	{
		StackAngle = PI / 2.0 - StackIndex * StackStep; // starting from pi/2 to -pi/2
		StackRadius = Data.Radius * FMath::Cos(StackAngle);
		StackHeight = Data.Radius * FMath::Sin(StackAngle);

		// Add (NumSectors + 1) vertices per stack. The first and last vertex
		// have same position and normal, but different tex coords.
		for (uint32 SectorIndex = 0; SectorIndex <= Data.Sectors;
			 ++SectorIndex) // sector = horizontal segment
		{
			SectorAngle = SectorIndex * SectorStep; // starting from 0 to 2pi

			Position.X = StackRadius * FMath::Cos(SectorAngle);
			Position.Y = StackRadius * FMath::Sin(SectorAngle);
			Position.Z = StackHeight;

			TangentZ =
				FVector(Position.X * RadiusInv, Position.Y * RadiusInv, Position.Z * RadiusInv);

			// vertex tex coord (u, v) range between [0, 1]
			TexCoord.X = (float) SectorIndex / Data.Sectors;
			TexCoord.Y = (float) StackIndex / Data.Stacks;

			/// \todo Compute correctly based on texcoords!
			TangentX = FVector::ZeroVector;
			TangentY = FVector::ZeroVector;

			// Fill actual buffers
			VertexBuffers.PositionVertexBuffer.VertexPosition(NextFreeVertex) = Position;
			VertexBuffers.ColorVertexBuffer.VertexColor(NextFreeVertex) = Color;
			VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(NextFreeVertex, 0, TexCoord);
			VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(
				NextFreeVertex, TangentX, TangentY, TangentZ);

			NextFreeVertex++;
		}
	}

	// Generate index list of sphere triangles
	int K1, K2;
	for (uint32 StackIndex = 0; StackIndex < Data.Stacks; ++StackIndex)
	{
		K1 = FirstVertex + StackIndex * (Data.Sectors + 1); // beginning of current stack
		K2 = K1 + Data.Sectors + 1; // beginning of next stack

		for (uint32 SectorIndex = 0; SectorIndex < Data.Sectors; ++SectorIndex, ++K1, ++K2)
		{
			// 2 triangles per sector excluding first and last stacks
			// K1 => K2 => K1+1
			if (StackIndex != 0)
			{
				IndexBuffer.Indices[NextFreeIndex++] = K1;
				IndexBuffer.Indices[NextFreeIndex++] = K1 + 1;
				IndexBuffer.Indices[NextFreeIndex++] = K2;
			}

			// K1+1 => K2 => K2+1
			if (StackIndex != (Data.Stacks - 1))
			{
				IndexBuffer.Indices[NextFreeIndex++] = K1 + 1;
				IndexBuffer.Indices[NextFreeIndex++] = K2 + 1;
				IndexBuffer.Indices[NextFreeIndex++] = K2;
			}
		}
	}

	check(NextFreeVertex - FirstVertex == Data.Vertices);
	check(NextFreeIndex - FirstIndex == Data.Indices);
}

AGX_MeshUtilities::CylinderConstructionData::CylinderConstructionData(
	float InRadius, float InHeight, uint32 InNumCircleSegments, uint32 InNumHeightSegments,
	const FLinearColor& InMiddleColor, const FLinearColor& InOuterColor)
	: Radius(InRadius)
	, Height(InHeight)
	, CircleSegments(InNumCircleSegments)
	, HeightSegments(InNumHeightSegments)
	, MiddleColor(InMiddleColor)
	, OuterColor(InOuterColor)
	, VertexRows(HeightSegments + 1)
	, VertexColumns(CircleSegments + 1)
	, Caps(2)
	, VertexRowsAndCaps(VertexRows + 2)
	, Vertices(VertexRowsAndCaps * VertexColumns)
	, Indices(HeightSegments * CircleSegments * 6 + Caps * (CircleSegments - 2) * 3)
{
}

void AGX_MeshUtilities::CylinderConstructionData::AppendBufferSizes(
	uint32& InOutNumVertices, uint32& InOutNumIndices) const
{
	InOutNumVertices += Vertices;
	InOutNumIndices += Indices;
}

void AGX_MeshUtilities::MakeCylinder(
	TArray<FVector>& Positions, TArray<FVector>& Normals, TArray<uint32>& Indices,
	const CylinderConstructionData& Data)
{
	if (Data.CircleSegments < 4 || Data.CircleSegments > uint32(TNumericLimits<uint16>::Max()) ||
		Data.HeightSegments < 1 || Data.HeightSegments > uint32(TNumericLimits<uint16>::Max()) ||
		Data.Radius < 1.0e-6 || Data.Height < 1.0e-6)
		return;

	const float SegmentSize = 2.0 * PI / Data.CircleSegments;
	const float RadiusInv = 1.0f / Data.Radius;

	Positions.Empty(Data.Vertices);
	Normals.Empty(Data.Vertices);
	Indices.Empty(Data.Indices);

	float X, Y, Z; // vertex position
#ifdef WITH_TEXCOORDS
	float U, V; // vertex texture coordinate
#endif

	// Generate vertex attributes.

	// Add vertices per horizontal row on the cylinder, with bottom and top vertex rows
	// duplicated for the caps, because they need different normals and tex coords.
	// The sequence of vertices are as follow:
	// Bottom Cap, Bottom Row, Bottom Row + 1, ..., Top Row - 1, Top Row, Top Cap
	for (uint32 CapsAndRowIndex = 0; CapsAndRowIndex < Data.VertexRowsAndCaps; ++CapsAndRowIndex)
	{
		const int CapIndex =
			CapsAndRowIndex == 0 ? 0 : (CapsAndRowIndex == Data.VertexRows + 1 ? 1 : -1);
		const bool IsCap = CapIndex != -1;
		const uint32 RowIndex = FMath::Clamp<int32>(CapsAndRowIndex - 1, 0, Data.VertexRows - 1);
		const float RowHeight = Data.Height * RowIndex / (Data.VertexRows - 1) - Data.Height * 0.5f;

		// Add Data.VertexColumns num vertices in a circle per vertex row. The first and last vertex
		// in the same row have same position and normal, but different tex coords.
		for (uint32 ColumnIndex = 0; ColumnIndex < Data.VertexColumns; ++ColumnIndex)
		{
			float ColumnAngle = ColumnIndex * SegmentSize;

			X = Data.Radius * FMath::Cos(ColumnAngle);
			Y = RowHeight;
			Z = Data.Radius * FMath::Sin(ColumnAngle);

			Positions.Add(FVector(X, Y, Z));

			switch (CapIndex)
			{
				case 0: // bottom cap
					Normals.Add(FVector(0.0f, -1.0f, 0.0f));
					break;
				case 1: // top cap
					Normals.Add(FVector(0.0f, 1.0f, 0.0f));
					break;
				default: // normal row
					Normals.Add(FVector(X * RadiusInv, 0.0f, Z * RadiusInv));
					break;
			}

#ifdef WITH_TEXCOORDS
			// vertex tex coord (u, v) range between [0, 1]
			U = IsCap ? X : ((float) ColumnIndex / Data.CircleSegments);
			V = IsCap ? Z : ((float) RowIndex / (Data.VertexRows - 1));
			TexCoords.Add(U);
			TexCoords.Add(V);
#endif
		}
	}

	// Generate triangle indexes for the side segments of the capsule.
	uint32 K0, K1;
	for (uint32 HeightSegmentIndex = 0; HeightSegmentIndex < Data.HeightSegments;
		 ++HeightSegmentIndex)
	{
		K0 = Data.VertexColumns +
			 HeightSegmentIndex * Data.VertexColumns; // first vertex in bottom vertex row of height
													  // segment (offset by cap)
		K1 = K0 + Data.VertexColumns; // first vertex in next row

		for (uint32 CircleSegmentIndex = 0; CircleSegmentIndex < Data.CircleSegments;
			 ++CircleSegmentIndex, ++K0, ++K1)
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
	uint32 K2;
	for (uint32 CapIndex = 0; CapIndex < Data.Caps; ++CapIndex)
	{
		K0 =
			CapIndex * (Data.VertexRowsAndCaps - 1) * Data.VertexColumns; // first vertex in cap row
		K1 = K0 + 1; // second vertex in cap row
		K2 = K1 + 1; // third vertex in cap row

		for (uint32 CircleSegmentIndex = 1; CircleSegmentIndex < Data.CircleSegments - 1;
			 ++CircleSegmentIndex, ++K1, ++K2)
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

	check(Indices.Num() == Data.Indices);
	check(Positions.Num() == Data.Vertices);
	check(Normals.Num() == Data.Vertices);
}

void AGX_MeshUtilities::MakeCylinder(
	FStaticMeshVertexBuffers& VertexBuffers, FDynamicMeshIndexBuffer32& IndexBuffer,
	uint32& NextFreeVertex, uint32& NextFreeIndex, const CylinderConstructionData& Data)
{
	check(Data.CircleSegments >= 4);
	check(Data.CircleSegments <= uint32(TNumericLimits<uint16>::Max()));
	check(Data.HeightSegments >= 1);
	check(Data.HeightSegments <= uint32(TNumericLimits<uint16>::Max()));
	check(Data.Radius >= 1.0e-6);
	check(Data.Height >= 1.0e-6);

	check(NextFreeVertex + Data.Vertices <= VertexBuffers.PositionVertexBuffer.GetNumVertices());
	check(NextFreeVertex + Data.Vertices <= VertexBuffers.StaticMeshVertexBuffer.GetNumVertices());
	check(NextFreeVertex + Data.Vertices <= VertexBuffers.ColorVertexBuffer.GetNumVertices());
	check(NextFreeIndex + Data.Indices <= static_cast<uint32>(IndexBuffer.Indices.Num()));

	const uint32 FirstVertex = NextFreeVertex;
	const uint32 FirstIndex = NextFreeIndex;

	const float SegmentSize = 2.0 * PI / Data.CircleSegments;
	const float RadiusInv = 1.0f / Data.Radius;

	FVector Position, TangentX, TangentY, TangentZ;
	FLinearColor Color; /// \todo Set vertex color to something.
	FVector2D TexCoord;

	// Generate vertex attributes.

	// Add vertices per horizontal row on the cylinder, with bottom and top vertex rows
	// duplicated for the caps, because they need different normals and tex coords.
	// The sequence of vertices are as follow:
	// Bottom Cap, Bottom Row, Bottom Row + 1, ..., Top Row - 1, Top Row, Top Cap
	for (uint32 CapsAndRowIndex = 0; CapsAndRowIndex < Data.VertexRowsAndCaps; ++CapsAndRowIndex)
	{
		const int32 CapIndex =
			CapsAndRowIndex == 0 ? 0 : (CapsAndRowIndex == Data.VertexRows + 1 ? 1 : -1);
		const bool IsCap = CapIndex != -1;
		const uint32 RowIndex = FMath::Clamp<int32>(CapsAndRowIndex - 1, 0, Data.VertexRows - 1);
		const float RowHeight = Data.Height * RowIndex / (Data.VertexRows - 1) - Data.Height * 0.5f;

		Color = FMath::Lerp(
			Data.MiddleColor, Data.OuterColor,
			FMath::Abs((static_cast<float>(RowIndex) / Data.HeightSegments) - 0.5f) * 2.0f);

		// Add Data.VertexColumns num vertices in a circle per vertex row. The first and last vertex
		// in the same row have same position and normal, but different tex coords.
		for (uint32 ColumnIndex = 0; ColumnIndex < Data.VertexColumns; ++ColumnIndex)
		{
			float ColumnAngle = ColumnIndex * SegmentSize;

			// Vertex Position
			Position.X = Data.Radius * FMath::Cos(ColumnAngle);
			Position.Y = Data.Radius * FMath::Sin(ColumnAngle);
			Position.Z = RowHeight;

			// Vertex Texture Coordinates, range between [0, 1]
			TexCoord.X = IsCap ? Position.X : ((float) ColumnIndex / Data.CircleSegments);
			TexCoord.Y = IsCap ? Position.Y : ((float) RowIndex / (Data.VertexRows - 1));

			// Vertex Normal, Tangent, and Binormal
			switch (CapIndex)
			{
				case 0: // bottom cap
					TangentZ = FVector(0.0f, 0.0f, -1.0f);
					break;
				case 1: // top cap
					TangentZ = FVector(0.0f, 0.0f, 1.0f);
					break;
				default: // normal row
					TangentZ = FVector(Position.X * RadiusInv, Position.Y * RadiusInv, 0.0f);
					break;
			}

			/// \todo Compute correctly based on texcoords!
			TangentX = FVector::ZeroVector;
			TangentY = FVector::ZeroVector;

			// Fill actual buffers
			VertexBuffers.PositionVertexBuffer.VertexPosition(NextFreeVertex) = Position;
			VertexBuffers.ColorVertexBuffer.VertexColor(NextFreeVertex) = Color.ToFColor(false);
			VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(NextFreeVertex, 0, TexCoord);
			VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(
				NextFreeVertex, TangentX, TangentY, TangentZ);

			NextFreeVertex++;
		}
	}

	// Generate triangle indexes for the side segments of the capsule.
	uint32 K0, K1;
	for (uint32 HeightSegmentIndex = 0; HeightSegmentIndex < Data.HeightSegments;
		 ++HeightSegmentIndex)
	{
		K0 = FirstVertex + Data.VertexColumns +
			 HeightSegmentIndex * Data.VertexColumns; // first vertex in bottom vertex row of height
													  // segment (offset by cap)
		K1 = K0 + Data.VertexColumns; // first vertex in next row

		for (uint32 CircleSegmentIndex = 0; CircleSegmentIndex < Data.CircleSegments;
			 ++CircleSegmentIndex, ++K0, ++K1)
		{
			// 2 triangles per segment (i.e. quad)

			IndexBuffer.Indices[NextFreeIndex++] = K0 + 1;
			IndexBuffer.Indices[NextFreeIndex++] = K0;
			IndexBuffer.Indices[NextFreeIndex++] = K1;

			IndexBuffer.Indices[NextFreeIndex++] = K1;
			IndexBuffer.Indices[NextFreeIndex++] = K1 + 1;
			IndexBuffer.Indices[NextFreeIndex++] = K0 + 1;
		}
	}

	// Generate triangle indexes for the caps, with triangle fan pattern.
	uint32 K2;
	for (uint32 CapIndex = 0; CapIndex < Data.Caps; ++CapIndex)
	{
		K0 = FirstVertex + CapIndex * (Data.VertexRowsAndCaps - 1) *
							   Data.VertexColumns; // first vertex in cap row
		K1 = K0 + 1; // second vertex in cap row
		K2 = K1 + 1; // third vertex in cap row

		for (uint32 CircleSegmentIndex = 1; CircleSegmentIndex < Data.CircleSegments - 1;
			 ++CircleSegmentIndex, ++K1, ++K2)
		{
			// 1 triangles per segment (except for first and last segment)

			if (CapIndex == 0)
			{
				IndexBuffer.Indices[NextFreeIndex++] = K0;
				IndexBuffer.Indices[NextFreeIndex++] = K1;
				IndexBuffer.Indices[NextFreeIndex++] = K2;
			}
			else
			{
				IndexBuffer.Indices[NextFreeIndex++] = K2;
				IndexBuffer.Indices[NextFreeIndex++] = K1;
				IndexBuffer.Indices[NextFreeIndex++] = K0;
			}
		}
	}

	check(NextFreeVertex - FirstVertex == Data.Vertices);
	check(NextFreeIndex - FirstIndex == Data.Indices);
}

void AGX_MeshUtilities::MakeCylinder(
	const FVector& Base, const FVector& XAxis, const FVector& YAxis, const FVector& ZAxis,
	float Radius, float HalfHeight, uint32 Sides, TArray<FDynamicMeshVertex>& OutVerts,
	TArray<uint32>& OutIndices)
{
	const float AngleDelta = 2.0f * PI / Sides;
	FVector LastVertex = Base + XAxis * Radius;

	FVector2D TC = FVector2D(0.0f, 0.0f);
	float TCStep = 1.0f / Sides;

	FVector TopOffset = HalfHeight * ZAxis;

	int32 BaseVertIndex = OutVerts.Num();

	// Compute vertices for base circle.
	for (uint32 SideIndex = 0; SideIndex < Sides; SideIndex++)
	{
		const FVector Vertex = Base + (XAxis * FMath::Cos(AngleDelta * (SideIndex + 1)) +
									   YAxis * FMath::Sin(AngleDelta * (SideIndex + 1))) *
										  Radius;
		FVector Normal = Vertex - Base;
		Normal.Normalize();

		FDynamicMeshVertex MeshVertex;

		MeshVertex.Position = Vertex - TopOffset;
		MeshVertex.TextureCoordinate[0] = TC;

		MeshVertex.SetTangents(-ZAxis, (-ZAxis) ^ Normal, Normal);

		OutVerts.Add(MeshVertex); // Add bottom vertex

		LastVertex = Vertex;
		TC.X += TCStep;
	}

	LastVertex = Base + XAxis * Radius;
	TC = FVector2D(0.0f, 1.0f);

	// Compute vertices for the top circle
	for (uint32 SideIndex = 0; SideIndex < Sides; SideIndex++)
	{
		const FVector Vertex = Base + (XAxis * FMath::Cos(AngleDelta * (SideIndex + 1)) +
									   YAxis * FMath::Sin(AngleDelta * (SideIndex + 1))) *
										  Radius;
		FVector Normal = Vertex - Base;
		Normal.Normalize();

		FDynamicMeshVertex MeshVertex;

		MeshVertex.Position = Vertex + TopOffset;
		MeshVertex.TextureCoordinate[0] = TC;

		MeshVertex.SetTangents(-ZAxis, (-ZAxis) ^ Normal, Normal);

		OutVerts.Add(MeshVertex); // Add top vertex

		LastVertex = Vertex;
		TC.X += TCStep;
	}

	// Add top/bottom triangles, in the style of a fan.
	// Note if we wanted nice rendering of the caps then we need to duplicate the vertices and
	// modify texture/tangent coordinates.
	for (uint32 SideIndex = 1; SideIndex < Sides; SideIndex++)
	{
		int32 V0 = BaseVertIndex;
		int32 V1 = BaseVertIndex + SideIndex;
		int32 V2 = BaseVertIndex + ((SideIndex + 1) % Sides);

		// bottom
		OutIndices.Add(V0);
		OutIndices.Add(V1);
		OutIndices.Add(V2);

		// top
		OutIndices.Add(Sides + V2);
		OutIndices.Add(Sides + V1);
		OutIndices.Add(Sides + V0);
	}

	// Add sides.

	for (uint32 SideIndex = 0; SideIndex < Sides; SideIndex++)
	{
		int32 V0 = BaseVertIndex + SideIndex;
		int32 V1 = BaseVertIndex + ((SideIndex + 1) % Sides);
		int32 V2 = V0 + Sides;
		int32 V3 = V1 + Sides;

		OutIndices.Add(V0);
		OutIndices.Add(V2);
		OutIndices.Add(V1);

		OutIndices.Add(V2);
		OutIndices.Add(V3);
		OutIndices.Add(V1);
	}
}

namespace
{
	FVector CalcConeVert(float Angle1, float Angle2, float AzimuthAngle)
	{
		float ang1 = FMath::Clamp<float>(Angle1, 0.01f, (float) PI - 0.01f);
		float ang2 = FMath::Clamp<float>(Angle2, 0.01f, (float) PI - 0.01f);

		float sinX_2 = FMath::Sin(0.5f * ang1);
		float sinY_2 = FMath::Sin(0.5f * ang2);

		float sinSqX_2 = sinX_2 * sinX_2;
		float sinSqY_2 = sinY_2 * sinY_2;

		float tanX_2 = FMath::Tan(0.5f * ang1);
		float tanY_2 = FMath::Tan(0.5f * ang2);

		float phi =
			FMath::Atan2(FMath::Sin(AzimuthAngle) * sinY_2, FMath::Cos(AzimuthAngle) * sinX_2);
		float sinPhi = FMath::Sin(phi);
		float cosPhi = FMath::Cos(phi);
		float sinSqPhi = sinPhi * sinPhi;
		float cosSqPhi = cosPhi * cosPhi;

		float rSq, r, Sqr, alpha, beta;

		rSq = sinSqX_2 * sinSqY_2 / (sinSqX_2 * sinSqPhi + sinSqY_2 * cosSqPhi);
		r = FMath::Sqrt(rSq);
		Sqr = FMath::Sqrt(1 - rSq);
		alpha = r * cosPhi;
		beta = r * sinPhi;

		FVector ConeVert;

		ConeVert.X = (1 - 2 * rSq);
		ConeVert.Y = 2 * Sqr * alpha;
		ConeVert.Z = 2 * Sqr * beta;

		return ConeVert;
	}
}

void AGX_MeshUtilities::MakeCone(
	float Angle1, float Angle2, float Scale, float XOffset, uint32 NumSides,
	TArray<FDynamicMeshVertex>& OutVerts, TArray<uint32>& OutIndices)
{
	TArray<FVector> ConeVerts;
	ConeVerts.AddUninitialized(NumSides);

	for (uint32 i = 0; i < NumSides; i++)
	{
		float Fraction = (float) i / (float) (NumSides);
		float Azi = 2.f * PI * Fraction;
		ConeVerts[i] = (CalcConeVert(Angle1, Angle2, Azi) * Scale) + FVector(XOffset, 0, 0);
	}

	for (uint32 i = 0; i < NumSides; i++)
	{
		// Normal of the current face
		FVector TriTangentZ = ConeVerts[(i + 1) % NumSides] ^ ConeVerts[i]; // aka triangle normal
		FVector TriTangentY = ConeVerts[i];
		FVector TriTangentX = TriTangentZ ^ TriTangentY;

		FDynamicMeshVertex V0, V1, V2;

		V0.Position = FVector(0) + FVector(XOffset, 0, 0);
		V0.TextureCoordinate[0].X = 0.0f;
		V0.TextureCoordinate[0].Y = (float) i / NumSides;
		V0.SetTangents(TriTangentX, TriTangentY, FVector(-1, 0, 0));
		int32 I0 = OutVerts.Add(V0);

		V1.Position = ConeVerts[i];
		V1.TextureCoordinate[0].X = 1.0f;
		V1.TextureCoordinate[0].Y = (float) i / NumSides;
		FVector TriTangentZPrev =
			ConeVerts[i] ^ ConeVerts[i == 0 ? NumSides - 1 : i - 1]; // Normal of the previous face
																	 // connected to this face
		V1.SetTangents(TriTangentX, TriTangentY, (TriTangentZPrev + TriTangentZ).GetSafeNormal());
		int32 I1 = OutVerts.Add(V1);

		V2.Position = ConeVerts[(i + 1) % NumSides];
		V2.TextureCoordinate[0].X = 1.0f;
		V2.TextureCoordinate[0].Y = (float) ((i + 1) % NumSides) / NumSides;
		FVector TriTangentZNext =
			ConeVerts[(i + 2) % NumSides] ^
			ConeVerts[(i + 1) % NumSides]; // Normal of the next face connected to this face
		V2.SetTangents(TriTangentX, TriTangentY, (TriTangentZNext + TriTangentZ).GetSafeNormal());
		int32 I2 = OutVerts.Add(V2);

		// Flip winding for negative scale
		if (Scale >= 0.f)
		{
			OutIndices.Add(I0);
			OutIndices.Add(I1);
			OutIndices.Add(I2);
		}
		else
		{
			OutIndices.Add(I0);
			OutIndices.Add(I2);
			OutIndices.Add(I1);
		}
	}
}

AGX_MeshUtilities::CylindricalArrowConstructionData::CylindricalArrowConstructionData(
	float InCylinderRadius, float InCylinderHeight, float InConeRadius, float InConeHeight,
	bool bInBottomCap, uint32 InNumCircleSegments, const FLinearColor& InBaseColor,
	const FLinearColor& InTopColor)
	: CylinderRadius(InCylinderRadius)
	, CylinderHeight(InCylinderHeight)
	, ConeRadius(InConeRadius)
	, ConeHeight(InConeHeight)
	, bBottomCap(bInBottomCap)
	, CircleSegments(InNumCircleSegments)
	, BaseColor(InBaseColor)
	, TopColor(InTopColor)
	, VertexRows(bBottomCap ? 7 : 6)
	, VertexColumns(CircleSegments + 1)
	, Vertices(VertexRows * VertexColumns)
	,
#ifdef CONE_SINGULARITY
	Indices(CircleSegments * (2 * 6 + 1 * 3) + (bBottomCap ? (CircleSegments - 2) * 3 : 0))
#else
	Indices(3 * CircleSegments * 6 + (bBottomCap ? (CircleSegments - 2) * 3 : 0))
#endif
{
}

void AGX_MeshUtilities::CylindricalArrowConstructionData::AppendBufferSizes(
	uint32& InOutNumVertices, uint32& InOutNumIndices) const
{
	InOutNumVertices += Vertices;
	InOutNumIndices += Indices;
};

void AGX_MeshUtilities::MakeCylindricalArrow(
	FStaticMeshVertexBuffers& VertexBuffers, FDynamicMeshIndexBuffer32& IndexBuffer,
	uint32& NextFreeVertex, uint32& NextFreeIndex, const CylindricalArrowConstructionData& Data)
{
	check(Data.CircleSegments >= 4);
	check(Data.CircleSegments <= uint32(TNumericLimits<uint16>::Max()));
	check(Data.CylinderRadius >= 1.0e-6);
	check(Data.CylinderHeight >= 1.0e-6);
	check(Data.ConeRadius >= 1.0e-6);
	check(Data.ConeHeight >= 1.0e-6);

	check(NextFreeVertex + Data.Vertices <= VertexBuffers.PositionVertexBuffer.GetNumVertices());
	check(NextFreeVertex + Data.Vertices <= VertexBuffers.StaticMeshVertexBuffer.GetNumVertices());
	check(NextFreeVertex + Data.Vertices <= VertexBuffers.ColorVertexBuffer.GetNumVertices());
	check(NextFreeIndex + Data.Indices <= static_cast<uint32>(IndexBuffer.Indices.Num()));

	const uint32 FirstVertex = NextFreeVertex;
	const uint32 FirstIndex = NextFreeIndex;

	const float SegmentSize = 2.0f * PI / Data.CircleSegments;
	const float CylinderRadiusInv = 1.0f / Data.CylinderRadius;
	const float ConeRadiusInv = 1.0f / Data.ConeRadius;
	const float ConeFlankLength =
		FMath::Sqrt(Data.ConeRadius * Data.ConeRadius + Data.ConeHeight * Data.ConeHeight);
	const float ConeFlankLengthOverHeight = ConeFlankLength / Data.ConeHeight;
	const float ConeRadiusOverFlankLength = Data.ConeRadius / ConeFlankLength;
	// const float ConeHeightOverRadius = Data.ConeHeight / Data.ConeRadius;
	// const float ConeRadiusOverHeight = Data.ConeRadius / Data.ConeHeight;
	const float TotalHeight = Data.CylinderHeight + Data.ConeHeight;

	FVector Position, TangentX, TangentY, TangentZ;
	FLinearColor Color; /// \todo Set vertex color to something.
	FVector2D TexCoord;

	// Generate vertex attributes.

	// Vertex rows needs to be duplicated for hard edges. Building following layout:
	//
	// At height == 0:
	//   1 vertex row for the cap ring (optional)
	//   1 vertex row for bottom of cylinder
	//
	// At height == cylinder height:
	//   1 vertex row for top of cylinder
	//   1 vertex row for inner base ring of cone (i.e. for a hole where cone intersects top of
	//   cylinder) 1 vertex row for outer base ring of cone
	//
	// At height == cylinder height + cone height:
	//   1 vertex row for outer base ring of cone
	//   1 vertex row for top of cone
	//
	for (uint32 RowIndex = 0; RowIndex < Data.VertexRows; ++RowIndex)
	{
		const bool IsCap = Data.bBottomCap && RowIndex == 0;
		const bool IsTopRow = RowIndex + 1 == Data.VertexRows;

		const uint32 HeightIndex =
			static_cast<uint32>(
				RowIndex >=
				static_cast<uint32>(Data.bBottomCap ? 2 : 1)) + // at or above cylinder top?
			static_cast<uint32>(
				RowIndex >= static_cast<uint32>(Data.bBottomCap ? 6 : 5)); // at cone top?

		const uint32 NormalIndex =
			static_cast<uint32>(
				RowIndex >=
				static_cast<uint32>(
					Data.bBottomCap ? 1 : 0)) + // at or above cylinder shell (pointing out)?
			static_cast<uint32>(
				RowIndex >=
				static_cast<uint32>(
					Data.bBottomCap ? 3 : 2)) + // at or above cone base (pointing down)?
			static_cast<uint32>(
				RowIndex >= static_cast<uint32>(
								Data.bBottomCap ? 5 : 4)); // at cone shell (pointing upwards-out)?

		const uint32 RadiusIndex =
			static_cast<uint32>(
				RowIndex >=
				static_cast<uint32>(Data.bBottomCap ? 4 : 3)) + // at or above cone outer base ring?
			static_cast<uint32>(
				RowIndex >= static_cast<uint32>(Data.bBottomCap ? 6 : 5)); // at cone top?

		const float RowHeight =
			(HeightIndex == 1 ? Data.CylinderHeight : (HeightIndex == 2 ? TotalHeight : 0.0f)) -
			0.5f * TotalHeight;
#ifdef CONE_SINGULARITY
		const float RowRadius =
			RadiusIndex == 0 ? Data.CylinderRadius : (RadiusIndex == 1 ? Data.ConeRadius : 0.0f);
#else
		const float RowRadius =
			RadiusIndex == 0 ? Data.CylinderRadius : (RadiusIndex == 1 ? Data.ConeRadius : 0.01f);
#endif

		Color =
			FMath::Lerp(Data.BaseColor, Data.TopColor, (float) RowIndex / (Data.VertexRows - 1));

		// Add Data.VertexColumns num vertices in a circle per vertex row. The first and last vertex
		// in the same row have same position and normal, but different tex coords.
		for (uint32 ColumnIndex = 0; ColumnIndex < Data.VertexColumns; ++ColumnIndex)
		{
#ifdef CONE_SINGULARITY
			const float ColumnAngle =
				float(ColumnIndex) * SegmentSize + (IsTopRow ? SegmentSize * 0.5f : 0.0f);
#else
			const float ColumnAngle = ColumnIndex * SegmentSize;
#endif
			// Vertex Position
			Position.X = RowRadius * FMath::Cos(ColumnAngle);
			Position.Y = RowRadius * FMath::Sin(ColumnAngle);
			Position.Z = RowHeight;

			// Vertex Texture Coordinates, range between [0, 1]
			TexCoord.X = IsCap ? Position.X : ((float) ColumnIndex / Data.CircleSegments);
			TexCoord.Y = IsCap ? Position.Y : ((float) RowIndex / (Data.VertexRows - 1));

			/// \todo Normals and TexCoords needs to be fixed for top ring!! Because Quads becomes
			/// Triangles!?

			// Vertex Normal, Tangent, and Binormal
			switch (NormalIndex)
			{
				case 0: // bottom cap
					TangentZ = FVector(0.0f, 0.0f, -1.0f);
					break;
				case 1: // cylinder shell
					TangentZ = FVector(
						Position.X * CylinderRadiusInv, Position.Y * CylinderRadiusInv, 0.0f);
					break;
				case 2: // cone base
					TangentZ = FVector(0.0f, 0.0f, -1.0f);
					break;
				case 3: // cone shell
						/// \todo This normal is not perfect! Fix for fact that quads becomes
						/// triangles on cone!
#ifdef CONE_SINGULARITY
					if (IsTopRow)
						TangentZ = FVector(0.0f, 0.0f, 1.0f);
					else
#endif
						TangentZ = FVector(
							FMath::Cos(ColumnAngle) * ConeFlankLengthOverHeight,
							FMath::Sin(ColumnAngle) * ConeFlankLengthOverHeight,
							ConeRadiusOverFlankLength);
					break;
					TangentZ.Normalize(); /// \todo Should not be needed. But normal is a bit too
										  /// long without it. Investigate!
					// TangentZ = FVector(Position.X * ConeRadiusInv * ConeHeightOverRadius,
					// Position.Y * ConeRadiusInv
					// * ConeHeightOverRadius, ConeRadiusOverHeight); break;
				default:
					check(!"AGX_MeshUtilities::MakeCylindricalArrow reached invalid NormalIndex");
					break;
			}

			/// \todo Compute correctly based on texcoords!
			TangentX = FVector::ZeroVector;
			TangentY = FVector::ZeroVector;

			// Fill actual buffers
			VertexBuffers.PositionVertexBuffer.VertexPosition(NextFreeVertex) = Position;
			VertexBuffers.ColorVertexBuffer.VertexColor(NextFreeVertex) = Color.ToFColor(false);
			VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(NextFreeVertex, 0, TexCoord);
			VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(
				NextFreeVertex, TangentX, TangentY, TangentZ);

			NextFreeVertex++;
		}
	}

	// Generate triangle indexes for the bottom cap, with triangle fan pattern.
	uint32 K0, K1, K2;
	if (Data.bBottomCap)
	{
		K0 = FirstVertex; // first vertex in cap row
		K1 = K0 + 1; // second vertex in cap row
		K2 = K1 + 1; // third vertex in cap row

		for (uint32 CircleSegmentIndex = 1; CircleSegmentIndex < Data.CircleSegments - 1;
			 ++CircleSegmentIndex, ++K1, ++K2)
		{
			// 1 triangles per segment (except for first and last segment)

			IndexBuffer.Indices[NextFreeIndex++] = K0;
			IndexBuffer.Indices[NextFreeIndex++] = K1;
			IndexBuffer.Indices[NextFreeIndex++] = K2;
		}
	}

	// Generate triangle indexes for the side segments of the arrow (can be thought of a cylinder
	// with many segments, where each segment has its own unique vertices)
	const uint32 OffsetByCapVertices = Data.bBottomCap ? Data.VertexColumns : 0;
	const uint32 NumHeightSegments = 3;
	for (uint32 HeightSegmentIndex = 0; HeightSegmentIndex < 3; ++HeightSegmentIndex)
	{
		K0 = FirstVertex + OffsetByCapVertices + HeightSegmentIndex * 2 * Data.VertexColumns;
		K1 = K0 + Data.VertexColumns; // first vertex in next row

		for (uint32 CircleSegmentIndex = 0; CircleSegmentIndex < Data.CircleSegments;
			 ++CircleSegmentIndex, ++K0, ++K1)
		{
			// 2 triangles per segment (i.e. quad), except for at top of cone (because quads turns
			// to triangles at top)

			IndexBuffer.Indices[NextFreeIndex++] = K0 + 1;
			IndexBuffer.Indices[NextFreeIndex++] = K0;
			IndexBuffer.Indices[NextFreeIndex++] = K1;

#ifdef CONE_SINGULARITY
			if (HeightSegmentIndex + 1 < NumHeightSegments)
#endif
			{
				IndexBuffer.Indices[NextFreeIndex++] = K1;
				IndexBuffer.Indices[NextFreeIndex++] = K1 + 1;
				IndexBuffer.Indices[NextFreeIndex++] = K0 + 1;
			}
		}
	}

	check(NextFreeVertex - FirstVertex == Data.Vertices);
	check(NextFreeIndex - FirstIndex == Data.Indices);
}

#undef CONE_SINGULARITY

AGX_MeshUtilities::BendableArrowConstructionData::BendableArrowConstructionData(
	float InRectangleWidth, float InRectangleLength, float InTriangleWidth, float InTriangleLength,
	float InBendAngle, uint32 InNumSegments, const FLinearColor& InBaseColor,
	const FLinearColor& InTopColor)
	: RectangleWidth(InRectangleWidth)
	, RectangleLength(InRectangleLength)
	, TriangleWidth(InTriangleWidth)
	, TriangleLength(InTriangleLength)
	, BendAngle(InBendAngle)
	, Segments(InNumSegments)
	, BaseColor(InBaseColor)
	, TopColor(InTopColor)
	, RectangleSegments(std::max<uint32>(
		  1, std::min<uint32>(
				 InNumSegments - 1, InNumSegments * InRectangleLength /
										static_cast<float>(InRectangleLength + InTriangleLength))))
	, TriangleSegments(std::max<uint32>(1, InNumSegments - RectangleSegments))
	, RectangleVertexRows(RectangleSegments + 1)
	, TriangleVertexRows(TriangleSegments + 1)
	, Vertices(2 * (RectangleVertexRows + TriangleVertexRows))
	, Indices(6 * (RectangleSegments + TriangleSegments))
{
}

void AGX_MeshUtilities::BendableArrowConstructionData::AppendBufferSizes(
	uint32& InOutNumVertices, uint32& InOutNumIndices) const
{
	InOutNumVertices += Vertices;
	InOutNumIndices += Indices;
}

void AGX_MeshUtilities::MakeBendableArrow(
	FStaticMeshVertexBuffers& VertexBuffers, FDynamicMeshIndexBuffer32& IndexBuffer,
	uint32& NextFreeVertex, uint32& NextFreeIndex, const BendableArrowConstructionData& Data)
{
	check(Data.Segments >= 2);
	check(Data.RectangleSegments >= 1);
	check(Data.TriangleSegments >= 1);
	check(Data.Segments == Data.TriangleSegments + Data.RectangleSegments);
	check(Data.RectangleWidth >= 1.0e-6);
	check(Data.RectangleLength >= 1.0e-6);
	check(Data.TriangleWidth >= 1.0e-6);
	check(Data.TriangleLength >= 1.0e-6);
	check(Data.BendAngle >= 0.0f);

	check(NextFreeVertex + Data.Vertices <= VertexBuffers.PositionVertexBuffer.GetNumVertices());
	check(NextFreeVertex + Data.Vertices <= VertexBuffers.StaticMeshVertexBuffer.GetNumVertices());
	check(NextFreeVertex + Data.Vertices <= VertexBuffers.ColorVertexBuffer.GetNumVertices());
	check(NextFreeIndex + Data.Indices <= static_cast<uint32>(IndexBuffer.Indices.Num()));

	const uint32 FirstVertex = NextFreeVertex;
	const uint32 FirstIndex = NextFreeIndex;

	const uint32 NumTotalRows = Data.RectangleVertexRows + Data.TriangleVertexRows;
	const uint32 NumTotalSegments = Data.RectangleSegments + Data.TriangleSegments;
	const float TotalLength = Data.RectangleLength + Data.TriangleLength;
	const bool IsBending = Data.BendAngle > 1.0e-6;
	const float BendRadius = IsBending ? TotalLength / Data.BendAngle : 0.0f;

	FVector Position, TangentX, TangentY, TangentZ;
	FLinearColor Color; /// \todo Set vertex color to something.
	FVector2D TexCoord;

	// Generate vertex attributes.
	for (uint32 RowIndex = 0; RowIndex < NumTotalRows; ++RowIndex)
	{
		const bool IsRectangle = RowIndex < Data.RectangleVertexRows;
		const uint32 RectangleRowIndexClamped =
			FMath::Clamp<int32>(RowIndex, 0, Data.RectangleVertexRows - 1);
		const uint32 TriangleRowIndexClamped = FMath::Clamp<int32>(
			RowIndex - Data.RectangleVertexRows, 0, Data.TriangleVertexRows - 1);

		const float RowDistance =
			Data.RectangleLength * RectangleRowIndexClamped / Data.RectangleSegments +
			Data.TriangleLength * TriangleRowIndexClamped / Data.TriangleSegments;

		const float RowNormalizedDistance = RowDistance / TotalLength;
		const float RowAngularDistance = IsBending ? RowNormalizedDistance * Data.BendAngle : 0.0f;

		const float CurrentWidth =
			IsRectangle
				? Data.RectangleWidth
				: FMath::Lerp(
					  Data.TriangleWidth, 0.01f,
					  TriangleRowIndexClamped / static_cast<float>(Data.TriangleVertexRows - 1));

		Color = FMath::Lerp(Data.BaseColor, Data.TopColor, RowNormalizedDistance);

		for (uint32 VertexIndexInRow = 0; VertexIndexInRow < 2; ++VertexIndexInRow)
		{
			const float RowExtentFactor = VertexIndexInRow == 0 ? -0.5f : 0.5f;

			// Vertex Position
			Position.X = IsBending ? FMath::Sin(-RowAngularDistance) * BendRadius : 0.0f;
			Position.Y = CurrentWidth * RowExtentFactor;
			Position.Z = IsBending ? FMath::Cos(RowAngularDistance) * BendRadius : RowDistance;

			// Vertex Texture Coordinates
			TexCoord.X = Position.Y;
			TexCoord.Y = RowDistance;

			// Vertex Normal, Tangent, and Binormal
			if (IsBending)
			{
				TangentZ = FVector(Position.X / BendRadius, 0.0f, Position.Z / BendRadius);
			}
			else
			{
				TangentZ = FVector(-1.0f, 0.0f, 0.0f);
			}

			/// \todo Compute correctly based on texcoords!
			TangentX = FVector::ZeroVector;
			TangentY = FVector::ZeroVector;

			// Fill actual buffers
			VertexBuffers.PositionVertexBuffer.VertexPosition(NextFreeVertex) = Position;
			VertexBuffers.ColorVertexBuffer.VertexColor(NextFreeVertex) = Color.ToFColor(false);
			VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(NextFreeVertex, 0, TexCoord);
			VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(
				NextFreeVertex, TangentX, TangentY, TangentZ);

			NextFreeVertex++;
		}
	}

	// Generate triangle indexes.
	for (uint32 SegmentIndex = 0, V0 = FirstVertex; SegmentIndex < NumTotalSegments;
		 ++SegmentIndex, V0 += 2)
	{
		if (SegmentIndex == Data.RectangleSegments)
		{
			V0 += 2; // Do not use last row of arrow's rectangle part for first quad in the arrow's
					 // triangle part.
		}

		// 2 triangles per segment (i.e. quad)

		IndexBuffer.Indices[NextFreeIndex++] = V0;
		IndexBuffer.Indices[NextFreeIndex++] = V0 + 3;
		IndexBuffer.Indices[NextFreeIndex++] = V0 + 1;

		IndexBuffer.Indices[NextFreeIndex++] = V0;
		IndexBuffer.Indices[NextFreeIndex++] = V0 + 2;
		IndexBuffer.Indices[NextFreeIndex++] = V0 + 3;
	}

	check(NextFreeVertex - FirstVertex == Data.Vertices);
	check(NextFreeIndex - FirstIndex == Data.Indices);
}

void AGX_MeshUtilities::PrintMeshToLog(
	const FStaticMeshVertexBuffers& VertexBuffers, const FDynamicMeshIndexBuffer32& IndexBuffer)
{
	const uint32 NumVertices = VertexBuffers.PositionVertexBuffer.GetNumVertices();
	const uint32 NumIndices = static_cast<uint32>(IndexBuffer.Indices.Num());

	UE_LOG(LogAGX, Log, TEXT("AGX_MeshUtilities::PrintMeshToLog() : Begin printing mesh data."))

	UE_LOG(LogAGX, Log, TEXT("      ----- Vertex Buffer -----"));
	for (uint32 VertexIndex = 0; VertexIndex < NumVertices; ++VertexIndex)
	{
		UE_LOG(
			LogAGX, Log, TEXT("  Vertex[%d] Position = <%s>, Normal = <%s>"), VertexIndex,
			*VertexBuffers.PositionVertexBuffer.VertexPosition(VertexIndex).ToString(),
			*VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(VertexIndex).ToString());
	}

	UE_LOG(LogAGX, Log, TEXT("      ----- Index Buffer -----"));
	for (uint32 Index = 0; Index < NumIndices; ++Index)
	{
		UE_LOG(
			LogAGX, Log, TEXT("  Index[%d] = %d \t(Position = <%s>, Normal = <%s>)"), Index,
			IndexBuffer.Indices[Index],
			*VertexBuffers.PositionVertexBuffer.VertexPosition(IndexBuffer.Indices[Index])
				 .ToString(),
			*VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(IndexBuffer.Indices[Index])
				 .ToString());
	}

	UE_LOG(LogAGX, Log, TEXT("AGX_MeshUtilities::PrintMeshToLog() : Finished printing mesh data."))
}

AGX_MeshUtilities::DiskArrayConstructionData::DiskArrayConstructionData(
	float InRadius, uint32 InNumCircleSegments, float InSpacing, uint32 InDisks, bool bInTwoSided,
	const FLinearColor InMiddleDiskColor, const FLinearColor InOuterDiskColor,
	TArray<FTransform> InSpacingsOverride)
	: Radius(InRadius)
	, CircleSegments(InNumCircleSegments)
	, Spacing(InSpacing)
	, Disks(InDisks)
	, bTwoSided(bInTwoSided)
	, MiddleDiskColor(InMiddleDiskColor)
	, OuterDiskColor(InOuterDiskColor)
	, SpacingsOverride(InSpacingsOverride)
	, SidesPerDisk(bInTwoSided ? 2 : 1)
	, VerticesPerSide(CircleSegments)
	, VerticesPerDisk(VerticesPerSide * SidesPerDisk)
	, Vertices(Disks * VerticesPerDisk)
	, Indices(Disks * 3 * (CircleSegments - 2) * SidesPerDisk)
{
}

void AGX_MeshUtilities::DiskArrayConstructionData::AppendBufferSizes(
	uint32& InOutNumVertices, uint32& InOutNumIndices) const
{
	InOutNumVertices += Vertices;
	InOutNumIndices += Indices;
}

void AGX_MeshUtilities::MakeDiskArray(
	FStaticMeshVertexBuffers& VertexBuffers, FDynamicMeshIndexBuffer32& IndexBuffer,
	uint32& NextFreeVertex, uint32& NextFreeIndex, const DiskArrayConstructionData& Data)
{
	check(Data.CircleSegments >= 4);
	check(Data.CircleSegments <= uint32(TNumericLimits<uint16>::Max()));
	check(Data.Disks >= 1);
	check(Data.Disks <= uint32(TNumericLimits<uint16>::Max()));
	check(Data.Radius >= 1.0e-6);

	check(NextFreeVertex + Data.Vertices <= VertexBuffers.PositionVertexBuffer.GetNumVertices());
	check(NextFreeVertex + Data.Vertices <= VertexBuffers.StaticMeshVertexBuffer.GetNumVertices());
	check(NextFreeVertex + Data.Vertices <= VertexBuffers.ColorVertexBuffer.GetNumVertices());
	check(NextFreeIndex + Data.Indices <= static_cast<uint32>(IndexBuffer.Indices.Num()));

	const uint32 FirstVertex = NextFreeVertex;
	const uint32 FirstIndex = NextFreeIndex;

	const float SegmentSize = 2.0 * PI / Data.CircleSegments;
	const float RadiusInv = 1.0f / Data.Radius;
	const float TotalHeight = Data.Spacing * Data.Disks;

	FVector Position, TangentX, TangentY, TangentZ;
	FLinearColor Color; /// \todo Set vertex color to something.
	FVector2D TexCoord;

	// Generate vertex attributes.
	for (uint32 DiskIndex = 0; DiskIndex < Data.Disks; ++DiskIndex)
	{
		const float NormalizedDiskIndex =
			DiskIndex / static_cast<float>(Data.Disks > 1 ? Data.Disks - 1 : 1);
		const float DiskHeight = TotalHeight * 0.5f - NormalizedDiskIndex * TotalHeight;

		Color = FMath::Lerp(
			Data.MiddleDiskColor, Data.OuterDiskColor,
			FMath::Abs(NormalizedDiskIndex - 0.5f) * 2.0f);

		for (uint32 SideIndex = 0; SideIndex < Data.SidesPerDisk; ++SideIndex)
		{
			for (uint32 CircleVertexIndex = 0; CircleVertexIndex < Data.CircleSegments;
				 ++CircleVertexIndex)
			{
				float CircleVertexAngle = CircleVertexIndex * SegmentSize;

				// Vertex Position
				Position.X = Data.Radius * FMath::Cos(CircleVertexAngle);
				Position.Y = Data.Radius * FMath::Sin(CircleVertexAngle);
				Position.Z = DiskHeight;

				// Vertex Texture Coordinates, range between [0, 1]
				TexCoord.X = Position.X;
				TexCoord.Y = Position.Y;

				// Vertex Normal, Tangent, and Binormal
				switch (SideIndex)
				{
					case 0: // up side
						TangentZ = FVector(0.0f, 0.0f, 1.0f);
						break;
					case 1: // down cap
					default:
						TangentZ = FVector(0.0f, 0.0f, -1.0f);
						break;
				}

				if (DiskIndex < static_cast<uint32>(Data.SpacingsOverride.Num()))
				{
					// Remove spacing and use the spacing defined in the array instead. May include
					// rotation as well.
					Position.Z = 0.0f;
					Position = Data.SpacingsOverride[DiskIndex].TransformPosition(Position);
					TangentZ = Data.SpacingsOverride[DiskIndex].TransformVector(TangentZ);
				}

				/// \todo Compute correctly based on texcoords!
				TangentX = FVector::ZeroVector;
				TangentY = FVector::ZeroVector;

				// Fill actual buffers
				VertexBuffers.PositionVertexBuffer.VertexPosition(NextFreeVertex) = Position;
				VertexBuffers.ColorVertexBuffer.VertexColor(NextFreeVertex) = Color.ToFColor(false);
				VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(NextFreeVertex, 0, TexCoord);
				VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(
					NextFreeVertex, TangentX, TangentY, TangentZ);

				NextFreeVertex++;
			}
		}
	}

	// Generate triangle indexes for each disk (and each side), with triangle fan pattern.
	uint32 V0, V1, V2;
	for (uint32 DiskIndex = 0; DiskIndex < Data.Disks; ++DiskIndex)
	{
		for (uint32 SideIndex = 0; SideIndex < Data.SidesPerDisk; ++SideIndex)
		{
			V0 = FirstVertex + SideIndex * Data.VerticesPerSide + DiskIndex * Data.VerticesPerDisk;

			// 1 triangle per segment except for first and last.
			for (uint32 CircleSegmentIndex = 1; CircleSegmentIndex < Data.CircleSegments - 1;
				 ++CircleSegmentIndex)
			{
				V1 = V0 + CircleSegmentIndex;
				V2 = V0 + ((CircleSegmentIndex + 1) % Data.CircleSegments);

				if (SideIndex == 0)
				{
					IndexBuffer.Indices[NextFreeIndex++] = V2;
					IndexBuffer.Indices[NextFreeIndex++] = V1;
					IndexBuffer.Indices[NextFreeIndex++] = V0;
				}
				else
				{
					IndexBuffer.Indices[NextFreeIndex++] = V0;
					IndexBuffer.Indices[NextFreeIndex++] = V1;
					IndexBuffer.Indices[NextFreeIndex++] = V2;
				}
			}
		}
	}

	check(NextFreeVertex - FirstVertex == Data.Vertices);
	check(NextFreeIndex - FirstIndex == Data.Indices);
}
