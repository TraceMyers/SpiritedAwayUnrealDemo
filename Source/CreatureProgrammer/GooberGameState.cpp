#include "GooberGameState.h"
#include "Goober.h"
#include "MacroHelpers.h"
#include "Sootsprite.h"
#include "Kismet/GameplayStatics.h"
#include "ThreadWorker.h"

AGooberGameState* AGooberGameState::Get(const UWorld* World)
{
	return CastChecked<AGooberGameState>(World->GetGameState());
}

AGooberGameState::AGooberGameState()
{
	CREATE_ROOT_COMPONENT()
	
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	FThreadWorker::ResetThreadNameIndex();
}

void AGooberGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void AGooberGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), GameConfig->TimeDilation);
	
	MonoTick(DeltaSeconds);
	DebugTick(DeltaSeconds);
}

void AGooberGameState::BeginPlay()
{
	Super::BeginPlay();
	
	Goober = GetWorld()->SpawnActor<AGoober>();
}

void AGooberGameState::MonoTick(float DeltaTime)
{
	for (ASootSprite* SootSprite : SootSprites)
	{
		SootSprite->TickUpdate(DeltaTime);
	}
	
	Goober->SootSpriteLimbIsm->TickUpdate(DeltaTime);
}

void AGooberGameState::DebugTick(float DeltaTime)
{
#if UE_BUILD_DEBUG | UE_BUILD_DEVELOPMENT
	if (GameConfig->Debug.bDrawLimbISMBounds)
	{
		Goober->SootSpriteLimbIsm->Debug_DrawBounds();
	}
	if (GameConfig->Debug.bDrawLimbMeshBounds)
	{
		Goober->SootSpriteLimbIsm->Debug_DrawLimbMeshBounds();
	}
	for (ASootSprite* SootSprite : SootSprites)
	{
		if (GameConfig->Debug.bDrawSootSpriteBounds)
		{
			SootSprite->Debug_DrawBounds();
		}
		if (GameConfig->Debug.bDrawSootSpriteVisionBox)
		{
			SootSprite->Debug_DrawVisionBox();
		}
	}
#endif
}
