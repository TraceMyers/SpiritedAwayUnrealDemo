#include "SootSpriteAI.h"

#include "SootSprite.h"
#include "MacroHelpers.h"

void FSootSpriteAI::TickUpdate(ASootSprite* InOwner, float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(ASootSprite_AI_TickUpdate)
	// sets ptr and nullifies pointer at end of scope, so no stale pointers, no need to involve gc.
	SET_SCOPED_OBJECT_POINTER(Body, InOwner)
	
	
}
