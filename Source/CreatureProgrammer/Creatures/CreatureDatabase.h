#pragma once

#include "CreatureProgrammer/Helpers/MacroHelpers.h"
#include "CreatureProgrammer/Helpers/SIMD.h"
#include "CreatureProgrammer/Multithreading/ThreadingTypes.h"
#include "UObject/UnrealType.h"
#include "CreatureDatabase.generated.h"

class ASootSprite;
class ACreature;

USTRUCT()
struct FSubCreatureArrayCollection
{
	GENERATED_BODY()
	
	DECLARE_GAME_THREAD_ONLY_ARRAY(ASootSprite*, SootSprites, true)
};

struct FCreatureBulkAIData
{
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
	
protected:
	
	void AddSubCreature(ACreature* Subcreature);
	
	DECLARE_GAME_THREAD_ONLY_ARRAY(ACreature*, Creatures, true)
	
public:
	
	UPROPERTY()
	FSubCreatureArrayCollection SubCreatures;
	
	FLender<FCreatureBulkAIData> AI;
	
};