#include "Goober.h"
#include "CreatureProgrammer/Creatures/SootSprite/SootSpriteLimb.h"
#include "CreatureProgrammer/Helpers/MacroHelpers.h"

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
