#include "GameConfig.h"
#include "GooberGameState.h"

UGameConfig* UGameConfig::Get(const UWorld* World)
{
	return CastChecked<AGooberGameState>(World->GetGameState())->GameConfig;
}

