#pragma once

#include "CoreMinimal.h"
#include "SootSpriteSettings.generated.h"

// settings for soot sprite creatures (SootSprite.h), stored on the game config 
// (GameConfig.h) data asset. used broadly as the starting place for all 
// individual parameters. On BeginPlay, each soot sprite takes a copy of
// this struct and randomizes it a bit to create individual variation.
USTRUCT()
struct FSootSpriteSettings
{
	GENERATED_BODY()
	
	// physics
	
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0.001))
	double Mass = 1.0;
	
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0.001))
	double LandingSpringStiffness = 2000;
	
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0, ClampMax=1))
	double LandingSpringDamping = 0.8;
	
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0.001))
	double LimbSpringStiffness = 400;
	
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0, ClampMax=1))
	double LimbSpringDamping = 0.9;
	
	// walk
	
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0, ClampMax=1))
	double Cowboy = 0.2;
	
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0, ClampMax=1))
	double WalkCycleRate = 1.25;
	
	// squash and stretch
	
	UPROPERTY(EditDefaultsOnly)
	double MaxStretch = 0.5;
	
	UPROPERTY(EditDefaultsOnly)
	double StretchSpeedMin = 300.0;
	
	UPROPERTY(EditDefaultsOnly)
	double StretchSpeedMax = 1000.0;
	
	UPROPERTY(EditDefaultsOnly)
	double StretchScaleSpeed = 12.0;
	
	// eyes
	
	UPROPERTY(EditDefaultsOnly)
	double PupilMoveSpeed = 50.0;
	
	UPROPERTY(EditDefaultsOnly)
	double EyesSwivelAcceleration = 300.0;
	
	UPROPERTY(EditDefaultsOnly)
	double EyesSwivelDeceleration = 600.0;
	
	UPROPERTY(EditDefaultsOnly)
	double EyesSwivelMaxSpeed = 400.0;
	
	UPROPERTY(EditDefaultsOnly)
	double EyesSwivelExcitementMultiplierMax = 8.0f;
	
	// limbs
	
	UPROPERTY(EditDefaultsOnly)
	UMaterial* LimbMaterial = nullptr;
	
	// personality
	
	UPROPERTY(EditDefaultsOnly)
	double ExcitementFloor = 0.2;
	
	UPROPERTY(EditDefaultsOnly)
	double ExcitingFallSpeedMin = 500.0;
	
	// misc
	
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0.0001))
	double SizeScale = 1.0;	
};