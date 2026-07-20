#pragma once

#include "CreatureProgrammer/Helpers/SIMD.h"
#include "CreatureDatabase.generated.h"

class ASootSprite;
class ACreature;

USTRUCT()
struct FCreatureDatabaseTransientData
{
	GENERATED_BODY()
	
	UPROPERTY()
	TArray<ASootSprite*> SootSprites;
	
	FSimdVector3Array CreatureLocations;
	FSimdVector3Array CreatureForwardVectors;
	
	TArray<double> CreatureVisibleSphereRadii;
	FSimdVector3Array CreatureVisionConeLocations;
	TArray<double> CreatureVisionCosHalfAngles;
	TArray<double> CreatureVisionMaxDistances;
};

// bulk creature data, stored per-frame
USTRUCT()
struct FCreatureDatabase
{
	GENERATED_BODY()
	
	void AddCreature(ACreature* Creature);
	
	void RemoveCreature(ACreature* Creature);
	
	void BeginTickUpdate();
	
	void EndTickUpdate();
	
	UPROPERTY()
	TArray<ACreature*>	Creatures;
	
	FCreatureDatabaseTransientData Transient;
};