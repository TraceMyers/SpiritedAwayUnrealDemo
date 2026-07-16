#pragma once

#include "Goober.generated.h"

// this actor is the global monolith for components to attach to if they want to render

class USootSpriteLimbISM;

UCLASS()
class CREATUREPROGRAMMER_API AGoober : public AActor
{
	GENERATED_BODY()
	
public:
	
	AGoober();
	
protected:
	
	virtual void BeginPlay() override;
	
public:
	
	UPROPERTY(VisibleInstanceOnly)
	USootSpriteLimbISM* SootSpriteLimbIsm;
	
protected:
	
public:
	
#if WITH_EDITORONLY_DATA
	
#endif
};