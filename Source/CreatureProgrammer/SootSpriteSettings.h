#pragma once

#include "CoreMinimal.h"
#include "SootSpriteSettings.generated.h"

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
	
	// UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0))
	// double WalkAcceleration = 300.0;
	//
	// UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0))
	// double WalkMaxSpeed = 400.0;
	//
	// UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0))
	// double CrouchWalkMaxSpeed = 900.0;
	
	// UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0))
	// double WalkGait = 220.0;
	
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0, ClampMax=1))
	double Cowboy = 0.2;
	
	// UPROPERTY(EditDefaultsOnly)
	// double ScootAcceleration = 900.0;
	//
	// UPROPERTY(EditDefaultsOnly)
	// double ScootMaxSpeed = 700.0;
	
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
	UMaterial* LimbMaterial;
	
	// personality
	
	UPROPERTY(EditDefaultsOnly)
	double ExcitementFloor = 0.2;
	
	// misc
	
	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0.0001))
	double SizeScale = 1.0;	
};