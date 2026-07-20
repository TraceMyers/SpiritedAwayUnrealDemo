#pragma once

#include "Creature.generated.h"

class UBoxComponent;

// flat bottom cone representing the vision of a creature
struct FCreatureVisionCone
{
	FVector RelativeOffset = FVector(0);
	double MaxDistance = 2500;
	double AngleDegrees = 100;
};

UCLASS()
class CREATUREPROGRAMMER_API ACreature : public AActor
{
	GENERATED_BODY()
	
public:
	
	ACreature();
	
	virtual void OnCreatureEnterVision(const ACreature* Creature) {}
	
	virtual void OnCreatureExitVision(const ACreature* Creature) {}
	
	FVector GetVisionOrigin() const
	{
		return GetActorLocation() + GetActorRotation().RotateVector(VisionCone.RelativeOffset);
	}
	
protected:
	
	virtual void BeginPlay() override;
	
public:
	
	UPROPERTY()
	TArray<ACreature*> VisibleCreatures;
	
	FCreatureVisionCone VisionCone;
	
	double VisualSphereRadius = 50.0;
	
protected:
	
	
public:
	
	// called if this creature is the game state's DebugCreature
	virtual void DebugMe();
	
};