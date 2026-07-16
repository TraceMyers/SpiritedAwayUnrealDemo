#pragma once

#include "Creature.generated.h"

class UBoxComponent;

UCLASS()

class CREATUREPROGRAMMER_API ACreature : public AActor
{
	GENERATED_BODY()
	
public:
	
	ACreature();
	
protected:
	
	UPROPERTY(EditInstanceOnly)
	UBoxComponent* VisionBox;
	
};