#include "Rock.h"

#include "MacroHelpers.h"

ARock::ARock()
{
	CREATE_ROOT_COMPONENT()
	
	CREATE_PRIMITIVE_COMPONENT(MeshCmp, .bCastShadow=true, .bVisible=true, .bSimulatePhysics=true)
}

void ARock::BeginPlay()
{
	Super::BeginPlay();
}
