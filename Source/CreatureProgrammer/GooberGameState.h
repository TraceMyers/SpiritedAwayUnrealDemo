#pragma once

#include "GameConfig.h"
#include "GameFramework/GameStateBase.h"
#include "GooberGameState.generated.h"

class ASootSprite;
class USootSpriteLimbISM;
class AGoober;

#define LIMB_ISM() USootSpriteLimbISM::Get(GetWorld())
#define GAME_CONFIG() UGameConfig::Get(GetWorld())
#define GAME_STATE()  AGooberGameState::Get(GetWorld())

UCLASS()
class CREATUREPROGRAMMER_API AGooberGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	
	static AGooberGameState* Get(const UWorld* World);
	
	AGooberGameState();
	
	virtual void PostInitializeComponents() override;
	
	virtual void Tick(float DeltaSeconds) override;
	
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
	TArray<ASootSprite*> SootSprites;
	
};

