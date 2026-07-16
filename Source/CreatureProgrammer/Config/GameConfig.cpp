#include "GameConfig.h"
#include "CreatureProgrammer/GameBase/GooberGameState.h"

UGameConfig* UGameConfig::Get(const UWorld* World)
{
	return CastChecked<AGooberGameState>(World->GetGameState())->GameConfig;
}

