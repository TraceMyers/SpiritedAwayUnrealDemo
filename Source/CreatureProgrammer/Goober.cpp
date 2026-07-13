#include "Goober.h"
#include "MacroHelpers.h"
#include "SootSpriteLimb.h"

AGoober::AGoober()
{
	CREATE_ROOT_COMPONENT()
	CREATE_PRIMITIVE_COMPONENT(SootSpriteLimbIsm, .bCastShadow=true, .bVisible=true)
	
	SootSpriteLimbIsm->bAffectDistanceFieldLighting = false;
	
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AGoober::BeginPlay()
{
	Super::BeginPlay();
	SootSpriteLimbIsm->Init();
}
