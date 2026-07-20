#include "CreatureDatabase.h"
#include "Creature.h"
#include "SootSprite/Sootsprite.h"

void FCreatureDatabase::AddCreature(ACreature* Creature)
{
	TArray<ACreature*>& Creatures_ThreadSafe = GetCreatures();
	check(!Creatures_ThreadSafe.Contains(Creature))
	Creatures_ThreadSafe.Add(Creature);
}

void FCreatureDatabase::RemoveCreature(ACreature* Creature)
{
	TArray<ACreature*>& Creatures_ThreadSafe = GetCreatures();
	const int32 CreatureIndex = Creatures_ThreadSafe.Find(Creature);
	check(CreatureIndex != 1)
	Creatures_ThreadSafe.RemoveAtSwap(CreatureIndex, 1, EAllowShrinking::No);
}

void FCreatureDatabase::BeginTickUpdate()
{
	PROFILE_FUNCTION()
	
	// todo: auto clear subcreature arrays
	TArray<ASootSprite*>& SootSprites = SubCreatures.GetSootSprites();
	SootSprites.Empty(SootSprites.Max());
	
	TArray<ACreature*>& Creatures_ThreadSafe = GetCreatures();
	
	check(AI.GetCounterValue() == 0)
	FLoan<FCreatureBulkAIData> AILoaner = AI.TakeLoan();
	FCreatureBulkAIData& AIData = *AILoaner.Data;
	AIData = {};
	
	for (int32 i = 0; i < Creatures_ThreadSafe.Num(); i++)
	{
		ACreature* Creature = Creatures_ThreadSafe[i];
		if (!IsValid(Creature))
		{
			Creatures_ThreadSafe.RemoveAtSwap(i);	
			i--;
			continue;
		}
		
		AddSubCreature(Creature);
		
		const FVector CreatureLocation = Creature->GetActorLocation();
		const FRotator CreatureRotation = Creature->GetActorRotation();
		
		AIData.CreatureLocations.Push(CreatureLocation);
		AIData.CreatureForwardVectors.Push(CreatureRotation.Vector());
		
		AIData.CreatureVisibleSphereRadii.Add(Creature->VisualSphereRadius);
		AIData.CreatureVisionConeLocations.Push(Creature->GetVisionOrigin());
		const double HalfAngleRadians = Creature->VisionCone.AngleDegrees * (0.5 * PI / 180.0);
		AIData.CreatureVisionCosHalfAngles.Add(FMath::Cos(HalfAngleRadians));
		AIData.CreatureVisionMaxDistances.Add(Creature->VisionCone.MaxDistance);
	}
}

void FCreatureDatabase::EndTickUpdate()
{
}

void FCreatureDatabase::AddSubCreature(ACreature* Subcreature)
{
	if (Subcreature->IsA(ASootSprite::StaticClass()))
	{
		TArray<ASootSprite*>& SootSprites = SubCreatures.GetSootSprites();
		SootSprites.Add(static_cast<ASootSprite*>(Subcreature));
	}
	else
	{
		checkf(false, L"non-creature type %s passed to AddSubCreature", *Subcreature->GetName())
	}
}
