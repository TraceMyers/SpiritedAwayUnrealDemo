#pragma once

typedef __m256d SimdVector;

namespace Simd
{
	FORCEINLINE TPair<int32, int32> VectorIndex(int32 i)
	{
		return {i / 4, i % 4};		
	}
	
	FORCEINLINE SimdVector Splat(double Value)
	{
		return _mm256_set1_pd(Value);
	}
	
	FORCEINLINE SimdVector Load(const double* Values)
	{
		return _mm256_load_pd(Values);		
	}
	
	FORCEINLINE void Store(SimdVector A, double* Values)
	{
		_mm256_store_pd(Values, A);
	}
	
	FORCEINLINE SimdVector Add(SimdVector A, SimdVector B)
	{
		return _mm256_add_pd(A, B);
	}

	FORCEINLINE SimdVector Sub(SimdVector A, SimdVector B)
	{
		return _mm256_sub_pd(A, B);
	}

	FORCEINLINE SimdVector Mul(SimdVector A, SimdVector B)
	{
		return _mm256_mul_pd(A, B);
	}
	
	FORCEINLINE SimdVector Div(SimdVector A, SimdVector B)
	{
		return _mm256_div_pd(A, B);
	}

	FORCEINLINE SimdVector MulAdd(SimdVector A, SimdVector B, SimdVector C)
	{
		return _mm256_fmadd_pd(A, B, C);
	}

	FORCEINLINE SimdVector MulSub(SimdVector A, SimdVector B, SimdVector C)
	{
		return _mm256_fmsub_pd(A, B, C);
	}
	
	FORCEINLINE SimdVector And(SimdVector A, SimdVector B)
	{
		return _mm256_and_pd(A, B);
	}
	
	FORCEINLINE SimdVector Or(SimdVector A, SimdVector B)
	{
		return _mm256_or_pd(A, B);
	}

	FORCEINLINE SimdVector Dot3D(SimdVector X1, SimdVector Y1, SimdVector Z1, SimdVector X2, SimdVector Y2, SimdVector Z2)
	{
		SimdVector XProduct = _mm256_mul_pd(X1, X2);
		SimdVector XYSum = _mm256_fmadd_pd(Y1, Y2, XProduct);
		return _mm256_fmadd_pd(Z1, Z2, XYSum);
	}

	FORCEINLINE SimdVector SizeSquared3D(SimdVector X, SimdVector Y, SimdVector Z)
	{
		const SimdVector XSq = _mm256_mul_pd(X, X);
		const SimdVector XYSq = _mm256_fmadd_pd(Y, Y, XSq);
		return _mm256_fmadd_pd(Z, Z, XYSq);
	}

	FORCEINLINE SimdVector Size3D(SimdVector X, SimdVector Y, SimdVector Z)
	{
		const SimdVector XSq = _mm256_mul_pd(X, X);
		const SimdVector XYSq = _mm256_fmadd_pd(Y, Y, XSq);
		const SimdVector XYZSq = _mm256_fmadd_pd(Z, Z, XYSq);
		return _mm256_sqrt_pd(XYZSq);
	}
	
	FORCEINLINE SimdVector Distance3D(SimdVector X1, SimdVector Y1, SimdVector Z1, SimdVector X2, SimdVector Y2, SimdVector Z2)
	{
		SimdVector XDiff = _mm256_sub_pd(X1, X2);
		SimdVector YDiff = _mm256_sub_pd(Y1, Y2);
		SimdVector ZDiff = _mm256_sub_pd(Z1, Z2);
		SimdVector XProduct = _mm256_mul_pd(XDiff, XDiff);
		SimdVector XYSum = _mm256_fmadd_pd(YDiff, YDiff, XProduct);
		return _mm256_fmadd_pd(ZDiff, ZDiff, XYSum);
	}
	
	FORCEINLINE SimdVector GreaterThanOrEqual(SimdVector A, SimdVector B)
	{
		return _mm256_cmp_pd(A, B, _CMP_GE_OQ);
	}
	
	FORCEINLINE SimdVector LessThanOrEqual(SimdVector A, SimdVector B)
	{
		return _mm256_cmp_pd(A, B, _CMP_LE_OQ);
	}
	
	FORCEINLINE SimdVector LessThan(SimdVector A, SimdVector B)
	{
		return _mm256_cmp_pd(A, B, _CMP_LT_OQ);
	}
	
	FORCEINLINE SimdVector GreaterThan(SimdVector A, SimdVector B)
	{
		return _mm256_cmp_pd(A, B, _CMP_GT_OQ);
	}
}

struct FSimdVector3Chunk
{
	double X[4] {0};
	double Y[4] {0};
	double Z[4] {0};
	static constexpr int32 VECTOR_SIZE = sizeof(X[0]) * 4;
};

struct FSimdVector3ArraySlice
{
	FORCEINLINE SimdVector LoadX(int32 Chunk) const
	{
		return Simd::Load(&Chunks[Chunk].X[0]);
	}
	
	FORCEINLINE SimdVector LoadY(int32 Chunk) const
	{
		return Simd::Load(&Chunks[Chunk].Y[0]);
	}
	
	FORCEINLINE SimdVector LoadZ(int32 Chunk) const
	{
		return Simd::Load(&Chunks[Chunk].Z[0]);
	}
	
	TConstArrayView<FSimdVector3Chunk> Chunks;
	const int32 First = 0;
	const int32 Count = 0;
};

struct FSimdVector3Array
{
	~FSimdVector3Array()
	{
		Chunks.Empty();		
	}
	
	void Push(const FVector& V)
	{
		const int32 InChunkIndex = Count & 0x3;
		const bool bSpaceRemaining = InChunkIndex != 0;
		if (!bSpaceRemaining)
		{
			Chunks.Add({});
			check(Chunks.Num() * 4 > Count + 1)
		}
		
		FSimdVector3Chunk& Chunk = Chunks[Chunks.Num()-1];
		Chunk.X[InChunkIndex] = V.X;
		Chunk.Y[InChunkIndex] = V.Y;
		Chunk.Z[InChunkIndex] = V.Z;
		
		Count++;
	}
	
	FORCEINLINE const FSimdVector3Chunk& GetChunk(int32 i)
	{
		return Chunks[i];
	}
	
	TConstArrayView<FSimdVector3Chunk> Slice(int32 i, int32 SliceCount) const
	{
		const int32 BeginChunkIndex = i / 4;
		const int32 ChunkNum = FMath::DivideAndRoundUp(SliceCount, 4);
		return TConstArrayView<FSimdVector3Chunk>(Chunks).Slice(BeginChunkIndex, ChunkNum);
	}
	
	FORCEINLINE SimdVector LoadX(int32 Chunk) const
	{
		return Simd::Load(&Chunks[Chunk].X[0]);
	}
	
	FORCEINLINE SimdVector LoadY(int32 Chunk) const
	{
		return Simd::Load(&Chunks[Chunk].Y[0]);
	}
	
	FORCEINLINE SimdVector LoadZ(int32 Chunk) const
	{
		return Simd::Load(&Chunks[Chunk].Z[0]);
	}
	
	// todo: less boilerplate
	
	FORCEINLINE SimdVector SplatX(int32 i)
	{
		const int32 Chunk = i / 4;
		const int32 Index = i % 4;
		return Simd::Splat(Chunks[Chunk].X[Index]);
	}
	
	FORCEINLINE SimdVector SplatY(int32 i)
	{
		const int32 Chunk = i / 4;
		const int32 Index = i % 4;
		return Simd::Splat(Chunks[Chunk].Y[Index]);
	}
	
	FORCEINLINE SimdVector SplatZ(int32 i)
	{
		const int32 Chunk = i / 4;
		const int32 Index = i % 4;
		return Simd::Splat(Chunks[Chunk].Z[Index]);
	}
	TArray<FSimdVector3Chunk, TAlignedHeapAllocator<FSimdVector3Chunk::VECTOR_SIZE>> Chunks;
	int32 Count = 0;
};
