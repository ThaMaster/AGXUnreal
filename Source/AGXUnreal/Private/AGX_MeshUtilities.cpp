// Fill out your copyright notice in the Description page of Project Settings.


#include "AGX_MeshUtilities.h"

#include "AGX_SimpleMeshComponent.h"
#include "UnrealMathUtility.h"


void AGX_MeshUtilities::MakeCube(TArray<FAGX_SimpleMeshTriangle>& Triangles, FVector HalfSize)
{
	TArray<FVector> Vertices;
	Vertices.Add(FVector(-HalfSize.X, -HalfSize.Y, -HalfSize.Z));
	Vertices.Add(FVector( HalfSize.X, -HalfSize.Y, -HalfSize.Z));
	Vertices.Add(FVector( HalfSize.X,  HalfSize.Y, -HalfSize.Z));
	Vertices.Add(FVector(-HalfSize.X,  HalfSize.Y, -HalfSize.Z));

	Vertices.Add(FVector(-HalfSize.X,  -HalfSize.Y,  HalfSize.Z));
	Vertices.Add(FVector( HalfSize.X, -HalfSize.Y,  HalfSize.Z));
	Vertices.Add(FVector( HalfSize.X,  HalfSize.Y,	HalfSize.Z));
	Vertices.Add(FVector(-HalfSize.X,  HalfSize.Y,	HalfSize.Z));
	
	int VertexIndicesPerTriangle[12][3] =
	{
		{0,1,2}, {0,2,3},
		{0,4,5}, {0,5,1},
		{1,5,6}, {1,6,2},
		{3,2,7}, {7,2,6},
		{0,3,4}, {4,3,7},
		{5,4,6}, {6,4,7}
	};

	for (auto &TriangleVertexIndices : VertexIndicesPerTriangle)
	{
		FAGX_SimpleMeshTriangle Triangle;
		Triangle.Vertex0 = Vertices[TriangleVertexIndices[0]];
		Triangle.Vertex1 = Vertices[TriangleVertexIndices[1]];
		Triangle.Vertex2 = Vertices[TriangleVertexIndices[2]];
		Triangles.Add(Triangle);
	}
}


void AGX_MeshUtilities::MakeSphere(TArray<FAGX_SimpleMeshTriangle>& Triangles, float Radius, uint32 NumSegments)
{
	if (NumSegments < 4 || Radius < 1.0e-6)
		return;

	const int32 NumStacks = NumSegments;
	const int32 NumSectors = NumStacks;

	const float SectorStep = 2.0 * PI / NumSectors;
	const float StackStep = PI / NumStacks;

	TArray<FVector> Vertices;
#ifdef WITH_NORMALS_AND_TEXCOORDS
	TArray<FVector> Normals;
	TArray<FVector2> TexCoords;
#endif

	float X, Y, Z;									// vertex position
#ifdef WITH_NORMALS_AND_TEXCOORDS
	float Nx, Ny, Nz, LengthInv = 1.0f / Radius;    // vertex normal
	float U, V;                                     // vertex texture coordinate
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

			// vertex position (x, y, z)
			X = StackRadius * FMath::Cos(SectorAngle);
			Y = StackRadius * FMath::Sin(SectorAngle);
			Z = StackHeight;
			Vertices.Add(FVector(X, Y, Z));

#ifdef WITH_NORMALS_AND_TEXCOORDS
			// normalized vertex normal (nx, ny, nz)
			Nx = X * LengthInv;
			Ny = Y * LengthInv;
			Nz = Z * LengthInv;
			Normals.push_back(Nx);
			Normals.push_back(Ny);
			Normals.push_back(Nz);

			// vertex tex coord (u, v) range between [0, 1]
			U = (float)SectorIndex / SectorCount;
			V = (float)StackIndex / StackCount;
			TexCoords.push_back(U);
			TexCoords.push_back(V);
#endif
		}
	}

#ifdef WITH_INDEX_BUFFER
	// Generate CW index list of sphere triangles
	std::vector<int> Indices;
#endif
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
#ifdef WITH_INDEX_BUFFER
				Indices.push_back(K1);
				Indices.push_back(K1 + 1);
				Indices.push_back(K2);
#else
				FAGX_SimpleMeshTriangle Triangle;
				Triangle.Vertex0 = Vertices[K1];
				Triangle.Vertex1 = Vertices[K1 + 1];
				Triangle.Vertex2 = Vertices[K2];
				Triangles.Add(Triangle);
#endif
			}

			// K1+1 => K2 => K2+1
			if (StackIndex != (NumStacks - 1))
			{
#ifdef WITH_INDEX_BUFFER
				Indices.push_back(K1 + 1);
				Indices.push_back(K2 + 1);
				Indices.push_back(K2);
#else

				FAGX_SimpleMeshTriangle Triangle;
				Triangle.Vertex0 = Vertices[K1 + 1];
				Triangle.Vertex1 = Vertices[K2 + 1];
				Triangle.Vertex2 = Vertices[K2];
				Triangles.Add(Triangle);
#endif
			}
		}
	}
}
