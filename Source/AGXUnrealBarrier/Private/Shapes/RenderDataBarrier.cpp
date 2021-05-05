#include "Shapes/RenderDataBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Shapes/RenderDataRef.h"
#include "TypeConversions.h"

FRenderDataBarrier::FRenderDataBarrier()
	: NativeRef(new FRenderDataRef())
{
}

FRenderDataBarrier::FRenderDataBarrier(FRenderDataBarrier&& Other)
	: NativeRef(std::move(Other.NativeRef))
{
}

FRenderDataBarrier::FRenderDataBarrier(std::unique_ptr<FRenderDataRef>&& InNativeRef)
	: NativeRef(std::move(InNativeRef))
{
}

FRenderDataBarrier::~FRenderDataBarrier()
{
}

namespace RenderDataBarrier_helpers
{
	static_assert(
		std::numeric_limits<std::size_t>::max() >= std::numeric_limits<int32>::max(),
		"Expecting std::size_t to hold all positive values that int32 can hold.");

	bool CheckSize(size_t Size)
	{
		const size_t MaxAllowed = static_cast<size_t>(std::numeric_limits<int32>::max());
		return Size <= MaxAllowed;
	}

	bool CheckSize(size_t Size, const TCHAR* DataName, const FGuid& Guid)
	{
		bool Ok = CheckSize(Size);
		if (!Ok)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Native Render Data %s contains more %s than Unreal can handle. Size is %zu."),
				*Guid.ToString(), DataName, Size);
		}
		return Ok;
	}

	int32 CastWithSaturate(size_t Size, const TCHAR* DataName, const FGuid& Guid)
	{
		if (CheckSize(Size, DataName, Guid))
		{
			return static_cast<int32>(Size);
		}
		else
		{
			return std::numeric_limits<int32>::max();
		}
	}

	template <typename AGXType, typename UnrealType, typename FGetAGXBuffer, typename FConvert>
	TArray<UnrealType> ConvertRenderBuffer(
		const FRenderDataBarrier& Barrier, const TCHAR* DataName, FGetAGXBuffer GetAgxBuffer,
		FConvert Convert)
	{
		TArray<UnrealType> DataUnreal;
		const agxCollide::RenderData* RenderData = Barrier.GetNative()->Native;
		if (RenderData == nullptr)
		{
			return DataUnreal;
		}
		const agx::VectorPOD<AGXType>& DataAGX = GetAgxBuffer(RenderData);
		const size_t SizeAGX = DataAGX.size();
		const int32 Size = CastWithSaturate(SizeAGX, DataName, Barrier.GetGuid());
		DataUnreal.Reserve(Size);
		for (const AGXType& DatumAGX : DataAGX)
		{
			DataUnreal.Add(Convert(DatumAGX));
		}
		return DataUnreal;
	}
}

bool FRenderDataBarrier::GetShouldRender() const
{
	check(HasNative());
	return NativeRef->Native->getShouldRender();
}

int32 FRenderDataBarrier::GetNumTriangles() const
{
	using namespace RenderDataBarrier_helpers;
	check(HasNative());
	const size_t NumIndicesAGX = NativeRef->Native->getIndexArray().size();
	if (NumIndicesAGX % 3 != 0)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Render Data with GUID %s has number of indices not divisible by three. It may "
				 "render incorrectly."),
			*GetGuid().ToString());
	}
	const int32 NumIndices = CastWithSaturate(NumIndicesAGX, TEXT("indices"), GetGuid());
	return NumIndices / 3;
}

int32 FRenderDataBarrier::GetNumIndices() const
{
	using namespace RenderDataBarrier_helpers;
	check(HasNative());
	const size_t NumIndicesAGX = NativeRef->Native->getIndexArray().size();
	return CastWithSaturate(NumIndicesAGX, TEXT("indices"), GetGuid());
}

TArray<FVector> FRenderDataBarrier::GetPositions() const
{
	return RenderDataBarrier_helpers::ConvertRenderBuffer<agx::Vec3, FVector>(
		*this, TEXT("positions"),
		[](const agxCollide::RenderData* Data) -> auto& { return Data->getVertexArray(); },
		[](const agx::Vec3& Vec3) { return ConvertDisplacement(Vec3); });
}

TArray<uint32> FRenderDataBarrier::GetIndices() const
{
	return RenderDataBarrier_helpers::ConvertRenderBuffer<agx::UInt32, uint32>(
		*this, TEXT("indices"),
		[](const agxCollide::RenderData* Data) -> auto& { return Data->getIndexArray(); },
		[](const agx::UInt32 Index) { return static_cast<uint32>(Index); });
}

TArray<FVector> FRenderDataBarrier::GetNormals() const
{
	return RenderDataBarrier_helpers::ConvertRenderBuffer<agx::Vec3, FVector>(
		*this, TEXT("normals"),
		[](const agxCollide::RenderData* Data) -> auto& { return Data->getNormalArray(); },
		[](const agx::Vec3& Vec3) { return ConvertVector(Vec3); });
}

TArray<FVector2D> FRenderDataBarrier::GetTextureCoordinates() const
{
	return RenderDataBarrier_helpers::ConvertRenderBuffer<agx::Vec2, FVector2D>(
		*this, TEXT("texture coordinates"),
		[](const agxCollide::RenderData* Data) -> auto& { return Data->getTexCoordArray(); },
		[](const agx::Vec2& Vec2) { return Convert(Vec2); });
}

FGuid FRenderDataBarrier::GetGuid() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getUuid());
}

bool FRenderDataBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}
FRenderDataRef* FRenderDataBarrier::GetNative()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return NativeRef.get();
}

const FRenderDataRef* FRenderDataBarrier::GetNative() const
{
	if (!HasNative())
	{
		return nullptr;
	}
	return NativeRef.get();
}

void FRenderDataBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef = nullptr;
}
