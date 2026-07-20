#pragma once

#include "Jobs/CreatureVisionUpdate.h"

// add a type to this template list to have it as an available job type. the type only needs a 
// 'void Execute(const UWorld* World);' procedure. 
typedef TVariant<
	FCreatureVisionUpdate
> FCreatureJobVariant;

enum class ECreatureJobLoad
{
	Full,
	Slice
};

class FCreatureJob
{
public:
	
	FCreatureJob() {}
	
	template <typename VariantType>
	static FCreatureJob CreateFull(const UWorld* World, const VariantType& InVariant)
	{
		FCreatureJob Job;
		Job.Variant.Set<VariantType>(InVariant);
		Job.Variant.Get<VariantType>().Prepare(World);
		Job.ExecuteProc = [World](FCreatureJobVariant& Variant)
		{
			Variant.Get<VariantType>().Execute(World);	
		};
		Job.ResultProc = [World](FCreatureJobVariant& Variant)
		{
			Variant.Get<VariantType>().ApplyResults(World);	
		};
		Job.ID = IDCounter.Increment();
		Job.Load = ECreatureJobLoad::Full;
		return Job;
	}
	
	template <typename VariantType>
	static FCreatureJob CreateSlice(const UWorld* World, const VariantType& InVariant, int32 First, int32 Num)
	{
		// first index being divisible by 4 makes simd jobs easier, since simd data are in groups of 4
		check(First % 4 == 0) 
		FCreatureJob Job;
		Job.Variant.Set<VariantType>(InVariant);
		Job.Variant.Get<VariantType>().Prepare(World, First, Num);
		Job.ExecuteProc = [World, First, Num](FCreatureJobVariant& Variant)
		{
			Variant.Get<VariantType>().Execute(World, First, Num);	
		};
		Job.ResultProc = [World, First, Num](FCreatureJobVariant& Variant)
		{
			Variant.Get<VariantType>().ApplyResults(World, First, Num);	
		};
		Job.ID = IDCounter.Increment();
		Job.Load = ECreatureJobLoad::Slice;
		return Job;
	}
	
	void Execute()
	{
		ExecuteProc(Variant);
	}
	
	void ApplyResults()
	{
		ResultProc(Variant);
	}
	
	int32 GetID() const { return ID; }
	
	const FCreatureJobVariant& GetVariant() { return Variant; }
	
protected:
	
	static FThreadSafeCounter IDCounter;
	
	int32 ID = -1;
	FCreatureJobVariant Variant;
	
	TFunction<void(FCreatureJobVariant& Variant)> ExecuteProc;
	TFunction<void(FCreatureJobVariant& Variant)> ResultProc;
	
	ECreatureJobLoad Load = ECreatureJobLoad::Full;
};
