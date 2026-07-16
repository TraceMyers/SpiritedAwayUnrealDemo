#include "Rock.h"

#include "MacroHelpers.h"

ARock::ARock()
{
	CREATE_ROOT_COMPONENT()
	
	CREATE_PRIMITIVE_COMPONENT_AS_ROOT(MeshCmp, .bCastShadow=true, .bVisible=true, .bSimulatePhysics=true, .CollisionEnabled=ECollisionEnabled::QueryAndPhysics)
}

void ARock::BeginPlay()
{
	Super::BeginPlay();
}
