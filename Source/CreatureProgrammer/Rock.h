#pragma once

#include "Rock.generated.h"

UCLASS()
class CREATUREPROGRAMMER_API ARock : public AActor
{
	GENERATED_BODY()
	
public:
	
	// unreal, construction and initialization
	
	ARock();
	
protected:
	
	virtual void BeginPlay() override;
	
public:
	
	// exposed uproperties
	
	UPROPERTY(VisibleDefaultsOnly)
	UStaticMeshComponent* MeshCmp;	
	
	// hidden uproperties
	
protected:
	
};