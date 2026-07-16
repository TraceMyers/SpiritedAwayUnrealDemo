#pragma once

#include "Jobs/CreatureVisionUpdate.h"

// add a type to this template list to have it as an available job type a job type only needs a 
// 'void Execute();' procedure. 
typedef TVariant<
	FCreatureVisionUpdate
> FCreatureJobVariant;

class FCreatureJob
{
public:
	
	template <typename VariantType>
	FCreatureJob(const VariantType& InVariant)
	{
		Variant.Set<VariantType>(InVariant);
		Procedure = [](FCreatureJobVariant& Variant)
		{
			Variant.Get<VariantType>().Execute();	
		};
		ID = IDCounter.Increment();
	}
	
	void Execute()
	{
		Procedure(Variant);
	}
	
	int32 GetID() const { return ID; }
	
	const FCreatureJobVariant& GetVariant() { return Variant; }
	
protected:
	
	static FThreadSafeCounter IDCounter;
	
	int32 ID;
	FCreatureJobVariant Variant;
	TFunction<void(FCreatureJobVariant& Variant)> Procedure;
};