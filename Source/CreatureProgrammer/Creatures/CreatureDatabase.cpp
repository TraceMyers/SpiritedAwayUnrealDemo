#include "CreatureDatabase.h"
#include "Creature.h"
#include "SootSprite/Sootsprite.h"

#define ADD_CREATURE(Array, VarName) \
	check(!Array.Contains(VarName)); \
	Array.Add(VarName);              \

#define CONDITIONAL_ADD_SUBCREATURE(Array, CreatureVarName)							\
do {																				\
	using Array##ElemT = TRemovePointer<decltype(Transient.Array)::ElementType>::Type;		\
	if (CreatureVarName->IsA<Array##ElemT>())										\
	{																				\
		Array##ElemT* Subcreature = static_cast<Array##ElemT*>(CreatureVarName);	\
		ADD_CREATURE(Transient.Array, Subcreature)											\
	}																				\
} while(0);

#define REMOVE_CREATURE(Array, VarName)					\
	const int32 VarName_ ## i = Array.Find(VarName);	\
	check(VarName_ ## i != -1);                         \
	Array.RemoveAtSwap(VarName_ ## i);

void FCreatureDatabase::AddCreature(ACreature* Creature)
{
	ADD_CREATURE(Creatures, Creature)
}

void FCreatureDatabase::RemoveCreature(ACreature* Creature)
{
	REMOVE_CREATURE(Creatures, Creature)
}

void FCreatureDatabase::BeginTickUpdate()
{
	PROFILE_FUNCTION()
	
	Transient = {};
	
	for (int32 i = 0; i < Creatures.Num(); i++)
	{
		ACreature* Creature = Creatures[i];
		if (!IsValid(Creature))
		{
			Creatures.RemoveAtSwap(i);	
			i--;
			continue;
		}
		
		CONDITIONAL_ADD_SUBCREATURE(SootSprites, Creature)
		
		const FVector CreatureLocation = Creature->GetActorLocation();
		const FRotator CreatureRotation = Creature->GetActorRotation();
		
		Transient.CreatureLocations.Push(CreatureLocation);
		Transient.CreatureForwardVectors.Push(CreatureRotation.Vector());
		
		Transient.CreatureVisibleSphereRadii.Add(Creature->VisualSphereRadius);
		Transient.CreatureVisionConeLocations.Push(Creature->GetVisionOrigin());
		const double HalfAngleRadians = Creature->VisionCone.AngleDegrees * (0.5 * PI / 180.0);
		Transient.CreatureVisionCosHalfAngles.Add(FMath::Cos(HalfAngleRadians));
		Transient.CreatureVisionMaxDistances.Add(Creature->VisionCone.MaxDistance);
	}
}

void FCreatureDatabase::EndTickUpdate()
{
}