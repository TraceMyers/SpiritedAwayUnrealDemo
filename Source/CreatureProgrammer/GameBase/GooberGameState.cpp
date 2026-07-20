#include "GooberGameState.h"
#include "Goober.h"
#include "CreatureProgrammer/Creatures/SootSprite/SootSprite.h"
#include "CreatureProgrammer/Helpers/MacroHelpers.h"
#include "CreatureProgrammer/Multithreading/ThreadWorker.h"
#include "CreatureProgrammer/Multithreading/JobSystem/CreatureJobs.h"
#include "Kismet/GameplayStatics.h"

AGooberGameState* AGooberGameState::Get(const UWorld* World)
{
	return CastChecked<AGooberGameState>(World->GetGameState());
}

AGooberGameState::AGooberGameState()
{
	CREATE_ROOT_COMPONENT()
	
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
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
	
	TickCount++;
}

void AGooberGameState::BeginPlay()
{
	Super::BeginPlay();
	
	Goober = GetWorld()->SpawnActor<AGoober>();
	
	FThreadWorker::ResetThreadNameIndex();
}

void AGooberGameState::MonoTick(float DeltaTime)
{
	CreatureDatabase.BeginTickUpdate();
	
	// todo: progressive slicing, put on execute on worker thread
	FCreatureJob Job = FCreatureJob::Create_Slice(GetWorld(), FCreatureVisionUpdate(), 0, CreatureDatabase.Creatures.Num());
	Job.Execute();
	Job.ApplyResults();
	
	for (ASootSprite* SootSprite : CreatureDatabase.Transient.SootSprites)
	{
		SootSprite->TickUpdate(DeltaTime);
	}
	
	Goober->SootSpriteLimbIsm->TickUpdate(DeltaTime);
	
	CreatureDatabase.EndTickUpdate();
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
	for (ASootSprite* SootSprite : CreatureDatabase.Transient.SootSprites)
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
	if (DebugCreature)
	{
		DebugCreature->DebugMe();
	}
#endif
}
