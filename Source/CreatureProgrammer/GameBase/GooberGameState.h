#pragma once

#include "CreatureProgrammer/Config/GameConfig.h"
#include "CreatureProgrammer/Creatures/CreatureDatabase.h"
#include "GameFramework/GameStateBase.h"
#include "GooberGameState.generated.h"

class ASootSprite;
class USootSpriteLimbISM;
class AGoober;

#define LIMB_ISM() USootSpriteLimbISM::Get(GetWorld())
#define GAME_CONFIG() UGameConfig::Get(GetWorld())
#define GAME_STATE()  AGooberGameState::Get(GetWorld())

#define WORLD_GAME_STATE(World)  AGooberGameState::Get(World)
#define WORLD_GAME_CONFIG(World) UGameConfig::Get(World)

UCLASS()
class CREATUREPROGRAMMER_API AGooberGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	
	static AGooberGameState* Get(const UWorld* World);
	
	AGooberGameState();
	
	virtual void PostInitializeComponents() override;
	
	virtual void Tick(float DeltaSeconds) override;
	
	// todo: organize this shitty data
	const FCreatureDatabase& GetCreatureDatabase() { return CreatureDatabase; }
	
	UPROPERTY()
	FCreatureDatabase CreatureDatabase;
	
protected:
	
	virtual void BeginPlay() override;
	
	// for controlling tick order easily. just manually tick some stuff.
	void MonoTick(float DeltaTime);
	
	void DebugTick(float DeltaTime);
	
public:
	
	UPROPERTY()
	AGoober* Goober;
	
	UPROPERTY(EditDefaultsOnly)
	UGameConfig* GameConfig;	
	
	UPROPERTY()
	ACreature* DebugCreature;
	
protected:
	
	int64 TickCount = 0;
};

