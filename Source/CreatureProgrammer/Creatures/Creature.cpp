#include "Creature.h"
#include "Components/BoxComponent.h"
#include "CreatureProgrammer/Helpers/MacroHelpers.h"

ACreature::ACreature()
{
	CREATE_PRIMITIVE_COMPONENT(VisionBox, .CollisionEnabled=ECollisionEnabled::QueryOnly)
	VisionBox->SetCollisionResponseToChannel(GameTraceChannel::SensorBox, ECR_Ignore);
	VisionBox->SetGenerateOverlapEvents(true);
}
