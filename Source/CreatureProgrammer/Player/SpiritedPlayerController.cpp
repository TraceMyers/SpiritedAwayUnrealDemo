#include "SpiritedPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "CreatureProgrammer/Creatures/Creature.h"
#include "CreatureProgrammer/GameBase/GooberGameState.h"
#include "CreatureProgrammer/Helpers/DebugDraw.h"
#include "CreatureProgrammer/Helpers/MacroHelpers.h"

ASpiritedPlayerController::ASpiritedPlayerController()
{
}

void ASpiritedPlayerController::DebugRaycast()
{
	GAME_STATE()->DebugCreature = nullptr;
	
	FColor HitColor = FColor::Cyan; // should never see cyan
		
	FHitResult HitResult;
	const FRay CamRay = GetCameraRay();
	if (GetWorld()->LineTraceSingleByChannel(HitResult, CamRay.Origin, CamRay.Origin + CamRay.Direction * 10000.0, GameTraceChannel::Creature))
	{
		if (!HitResult.GetActor())
		{
			UE_LOG(LogTemp, Error, L"hit a non-actor on the creature channel")
			HitColor = FColor::Magenta;
		}
		else
		{
			ACreature* Creature = Cast<ACreature>(HitResult.GetActor());
			if (Creature == nullptr)
			{
				UE_LOG(LogTemp, Error, L"hit an actor of type %s on the creature channel", *HitResult.GetActor()->GetClass()->GetName())
				HitColor = FColor::Yellow;
			}
			else
			{
				GAME_STATE()->DebugCreature = Creature;
				HitColor = FColor::Green;
			}
		}
	}
	
	HitResult.TraceStart += CamRay.Direction * 100;
	SpiritedDebugDraw::HitResult(GetWorld(), HitResult, 3, FColor::Red, HitColor);
}

FRay ASpiritedPlayerController::GetMouseCursorRay() const
{
	FRay Ray;
	DeprojectMousePositionToWorld(Ray.Origin, Ray.Direction);
	return Ray;
}

FRay ASpiritedPlayerController::GetCameraRay() const
{
	FRay Ray;
	FRotator CamRot;
	GetPlayerViewPoint(Ray.Origin, CamRot);
	Ray.Direction = CamRot.Vector();
	return Ray;
}

void ASpiritedPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
	
	EnhancedInput->BindAction(IA_DebugRaycast, ETriggerEvent::Started, this, &ASpiritedPlayerController::DebugRaycast);
}

void ASpiritedPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (auto* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		InputSubsystem->AddMappingContext(InputMappingContext, 0);
	}
}

