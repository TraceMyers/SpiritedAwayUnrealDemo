#pragma once

#include "SpiritedPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class CREATUREPROGRAMMER_API ASpiritedPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	
	ASpiritedPlayerController();
	
	void DebugRaycast();
	
	FRay GetMouseCursorRay() const;
	
	FRay GetCameraRay() const;
	
	UPROPERTY(EditAnywhere, Category="Input")
	UInputMappingContext* InputMappingContext;
	
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* IA_DebugRaycast;
	
protected:
	
	virtual void BeginPlay() override;
	
	virtual void SetupInputComponent() override;
	
};
