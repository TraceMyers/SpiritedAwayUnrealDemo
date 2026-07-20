#include "Creature.h"

#include "CreatureProgrammer/Helpers/MacroHelpers.h"

ACreature::ACreature()
{
}

void ACreature::BeginPlay()
{
	Super::BeginPlay();
}

void ACreature::DebugMe()
{
	const double AngleRadians = VisionCone.AngleDegrees * (0.5 * PI / 180.0); 
	// for some reason unreal has you provide the cone side length rather than the height...
	const double ConeSideLength = VisionCone.MaxDistance / FMath::Cos(AngleRadians);
	
	DrawDebugCone(GetWorld(), GetVisionOrigin(), GetActorRotation().Vector(), ConeSideLength, AngleRadians, AngleRadians, 16, FColor::Emerald);
	
	DrawDebugLine(GetWorld(), GetVisionOrigin(), GetVisionOrigin() + GetActorRotation().Vector() * VisionCone.MaxDistance, FColor::Red);
	
	for (ACreature* Creature : VisibleCreatures)
	{
		DrawDebugLine(GetWorld(), GetActorLocation(), Creature->GetActorLocation(), FColor::Blue);
		DrawDebugSphere(GetWorld(), Creature->GetActorLocation(), Creature->VisualSphereRadius, 16, FColor::Cyan);
	}
}
