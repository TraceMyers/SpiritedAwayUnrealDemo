#pragma once

#include "SootSpriteSettings.h"
#include "DebugSettings.h"
#include "GameConfig.generated.h"

UCLASS()
class CREATUREPROGRAMMER_API UGameConfig : public UDataAsset
{
	GENERATED_BODY()	
	
public:
	
	static UGameConfig* Get(const UWorld* World);
	
	UPROPERTY(EditDefaultsOnly, Category="General")
	double TimeDilation = 1.0;
	
	UPROPERTY(EditDefaultsOnly, Category="General")
	double Gravity = 980.0;
	
	UPROPERTY(EditDefaultsOnly, Category="SootSprite", meta=(ShowOnlyInnerProperties))
	FSootSpriteSettings SootSprite;	
	
	UPROPERTY(EditDefaultsOnly, Category="Debug", meta=(ShowOnlyInnerProperties))
	FDebugSettings Debug;
};