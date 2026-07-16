#include "SootSpriteAI.h"

#include "SootSprite.h"

void FSootSpriteAI::TickUpdate(ASootSprite* InOwner, float DeltaTime)
{
	PROFILE_FUNCTION()
	// sets ptr and nullifies pointer at end of scope, so no stale pointers, no need to involve gc.
	SET_SCOPED_OBJECT_POINTER(Body, InOwner)
}
