#pragma once
#include "CreatureProgrammer/Helpers/SIMD.h"


struct FCreatureVisionUpdate
{
	void Prepare(const UWorld* World, int32 First, int32 Count);
	void Execute(const UWorld* World, int32 First, int32 Count);
	void ApplyResults(const UWorld* World, int32 First, int32 Count);
	
protected:
	
	// input
	TConstArrayView<FSimdVector3Chunk> ForwardVectors;
	TConstArrayView<FSimdVector3Chunk> VisionOrigins;
	TConstArrayView<double> CosVisionAngles;
	TConstArrayView<double> VisionDistances;
	
	// output
	TArray<TPair<int16, int16>> WhoSeesWho;
};
