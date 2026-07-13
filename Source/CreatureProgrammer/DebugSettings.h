#pragma once

#include "DebugSettings.generated.h"

USTRUCT()
struct FDebugSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditInstanceOnly)
	bool bDrawLimbISMBounds = false;	
	
	UPROPERTY(EditInstanceOnly)
	bool bDrawLimbMeshBounds = false;
	
	UPROPERTY(EditInstanceOnly)
	bool bSootSpritesHaveNoXyVelocity = false;
	
	UPROPERTY(EditInstanceOnly)
	double SootSpriteAdditionalCarryingMass = 0;
	
	UPROPERTY(EditInstanceOnly)
	bool bDrawSootSpriteBounds = false;
	
	UPROPERTY(EditInstanceOnly)
	bool bDrawSootSpriteGroundRaycast = false;
	
	UPROPERTY(EditInstanceOnly)
	bool bDrawSootSpriteFootHeightRaycast = false;
	
	UPROPERTY(EditInstanceOnly)
	bool bDrawSootSpriteVisionBox = false;
};