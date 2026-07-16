#pragma once

#include "CoreMinimal.h"
#include "SootSpriteAI.generated.h"

class ASootSprite;
USTRUCT()
struct FSootSpriteAI
{
	GENERATED_BODY()
	
	void TickUpdate(ASootSprite* InOwner, float DeltaTime);
	
protected:
	
	ASootSprite* Body = nullptr;
};