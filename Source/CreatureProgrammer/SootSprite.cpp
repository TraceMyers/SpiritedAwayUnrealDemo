#include "Sootsprite.h"
#include "MacroHelpers.h"
#include "CreatureMath.h"
#include "DebugDraw.h"
#include "GooberGameState.h"
#include "GameConfig.h"
#include "SootSpriteLimb.h"
#include "Components/BoxComponent.h"
#include "Components/MaterialBillboardComponent.h"

double ASootSprite::BaseSpriteSize;
FAlphaSampler ASootSprite::WalkLimbHeightSampler;

#define BOTH_LEGS_DO(DoThing) \
	do { \
		USootSpriteLimb* CurrentLimb = GetLimb(LeftLeg); \
		CurrentLimb->DoThing; \
		CurrentLimb = GetLimb(RightLeg); \
		CurrentLimb->DoThing; \
	} while (0);

// pre-video checklist:
// x debug radius
// x remake walk to use ik
// - dust cloud on hard landing
// - excitement causes faster head swivel
// - excitement causes bigger/taller eyes
// - excitement causes more linear speed and linear accel
// - eyes look about (pupils scanning/looking at nothing in particular + look target algo / motivations)
// - feet
// - fix low-to-ground walk
// - moving faster turns into a run
// - too much weight = go into squish mode
// x after landing, do a dradle-ish rotation-settle
// x squish mode -> walk mode transition once unmoving
// - look target aversion (scaling)
// - concrete floor
// - fire light (+ affects soot sprites. maybe shrunken circle faked directional?)
// x put some of the rocks in the bg
// - UI to control the video demo
// x fix squash and stretch. squash sticks
// - have the good sense to clean up the code before putting it online.

ASootSprite::ASootSprite()
{
	CREATE_PRIMITIVE_COMPONENT_AS_ROOT(BodyMesh, .bSimulatePhysics=true)
	CREATE_PRIMITIVE_COMPONENT(EyesMesh, .bCastShadow=true, .bVisible=true)
	CREATE_PRIMITIVE_COMPONENT(BodySprite, .bVisible=true)
	CREATE_PRIMITIVE_COMPONENT(VisionBox)
	Limbs.SetNum(4);
	CREATE_ACTOR_COMPONENT(Limbs[0])
	CREATE_ACTOR_COMPONENT(Limbs[1])
	CREATE_ACTOR_COMPONENT(Limbs[2])
	CREATE_ACTOR_COMPONENT(Limbs[3])
	Toes.SetNum(6);
	CREATE_ACTOR_COMPONENT(Toes[0])
	CREATE_ACTOR_COMPONENT(Toes[1])
	CREATE_ACTOR_COMPONENT(Toes[2])
	CREATE_ACTOR_COMPONENT(Toes[3])
	CREATE_ACTOR_COMPONENT(Toes[4])
	CREATE_ACTOR_COMPONENT(Toes[5])
	
	EyesMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodySprite->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	
	EyesMesh->SetMobility(EComponentMobility::Movable);
	BodySprite->SetMobility(EComponentMobility::Movable);
	EyesMesh->SetAbsolute(false, false, false);
	BodySprite->SetAbsolute(false, false, false);
	
	BodyMesh->SetEnableGravity(true);
	BodyMesh->BodyInstance.bLockRotation = true;
	BodyMesh->BodyInstance.bLockXRotation = true;
	BodyMesh->BodyInstance.bLockYRotation = true;
	BodyMesh->BodyInstance.bLockZRotation = true;
	BodyMesh->BodyInstance.SetDOFLock(EDOFMode::SixDOF);
	
	VisionBox->SetRelativeScale3D(FVector(10));
	
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void ASootSprite::BeginPlay()
{
	Super::BeginPlay();
	
	if (BaseSpriteSize == 0)
	{
		BaseSpriteSize = BodySprite->Elements[0].BaseSizeX;
		WalkLimbHeightSampler = {};
		
		WalkLimbHeightSampler.GenerateSamples(12, [](float Alpha)
		{
			constexpr float THE_ANSWER = 42.0f; // apex height
			constexpr float SD_COUNT = 3.0f;
			const float SD = 1.0f / (SD_COUNT * 2);
			const float PdfMin = CreatureMath::SampleNormal(0.5f, SD, 0.0f);
			const float PdfMax = CreatureMath::SampleNormal(0.5f, SD, 0.5f);
			const float NormalSample = CreatureMath::SampleNormal(0.5f, SD, Alpha);
			return FMath::GetMappedRangeValueClamped(FVector2f(PdfMin, PdfMax), {0, THE_ANSWER}, NormalSample);
		});
	}
	
	BodySprite->CreateAndSetMaterialInstanceDynamic(0);
	EyesMesh->CreateAndSetMaterialInstanceDynamic(0);
	
	FAttachmentTransformRules AttachRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepWorld, false);
	
	BodySprite->AttachToComponent(BodyMesh, AttachRules);
	BodySprite->SetRelativeLocation(FVector(0));
	EyesMesh->AttachToComponent(BodyMesh, AttachRules);
	EyesMesh->SetRelativeLocation(FVector(0));
	
	GetBodySpriteMaterial()->SetScalarParameterValue(L"Index", FMath::RandRange(0, 3));
	GetEyesMaterial()->SetScalarParameterValue(L"Index", FMath::RandRange(0, 7));
	
	AGooberGameState* GameState = GAME_STATE();
	GameState->SootSprites.Add(this);
	
	Settings = GAME_CONFIG()->SootSprite;
	Settings.Mass = FMath::FRandRange(Settings.Mass * 0.5, Settings.Mass * 2.0);
	BodyMesh->SetMassOverrideInKg(NAME_None, Settings.Mass);
	
	const double MassDiff = Settings.Mass - GAME_CONFIG()->SootSprite.Mass;
	const double MassDiffSign = FMath::Sign(MassDiff);
	const double CubeRootMassDifference = FMath::Pow(MassDiff * MassDiffSign, 1.0/3.0);
	const double SqrtMassRatio = FMath::Sqrt(Settings.Mass / GAME_CONFIG()->SootSprite.Mass);
	
	Settings.WalkCycleRate = FMath::FRandRange(Settings.WalkCycleRate * 0.9, Settings.WalkCycleRate * 1.1) / SqrtMassRatio;
	// Settings.WalkAcceleration = FMath::FRandRange(Settings.WalkAcceleration * 0.9, Settings.WalkAcceleration * 1.1) / SqrtMassRatio;
	
	constexpr double DENSITY = 0.1;
	Settings.SizeScale += CubeRootMassDifference * MassDiffSign * DENSITY; // radius is proportional to cubert(mass)
	
	SetBodyScale(FVector2D(Settings.SizeScale));
	
	Settings.ExcitementFloor = FMath::FRandRange(Settings.ExcitementFloor, FMath::Min(Settings.ExcitementFloor + 0.2, 1));
	Excitement = Settings.ExcitementFloor;
	
	const double Radius = GetVisualRadius();
	
	for (USootSpriteLimb* Limb : Limbs)
	{
		Limb->AttachTarget.Type = ELimbAttachTargetType::World;
	}
	
	GetLimb(LeftLeg)->AttachOrigin = {this, FVector(-Radius * 0.18, -Radius * 0.4, -Radius * 0.8)};
	GetLimb(RightLeg)->AttachOrigin = {this, FVector(-Radius * 0.18, Radius * 0.4, -Radius * 0.8)};
	GetLimb(LeftArm)->AttachOrigin = {this, FVector(-Radius * 0.18, -Radius * 0.4, Radius * 0.8)};
	GetLimb(RightArm)->AttachOrigin = {this, FVector(-Radius * 0.18, Radius * 0.4, Radius * 0.8)};
	
	for (USootSpriteLimb* Toe : Toes)
	{
		Toe->AttachOrigin.Type = ELimbAttachTargetType::World;
		Toe->AttachTarget.Type = ELimbAttachTargetType::World;
		Toe->TargetLength = GetToeLength();
	}
	
	VisionBox->OnComponentBeginOverlap.AddDynamic(this, &ASootSprite::OnVisionBoxOverlap);
}

void ASootSprite::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AGooberGameState* GameState = Cast<AGooberGameState>(GetWorld()->GetGameState()))
	{
		GameState->SootSprites.Remove(this);
	}
	Super::EndPlay(EndPlayReason);
}

void ASootSprite::TickUpdate(float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(ASootSprite_TickUpdate)
	
	if (bDisableUpdate)
	{
		return;
	}
	
	// avoid crazy forces
	DeltaTime = FMath::Clamp(DeltaTime, 1.0f/480.0f, 1.0f/30.0f);
	
	// logic (nothing rn)
	AI.TickUpdate(this, DeltaTime);
	
	// physics
	double GroundDist = GetDistanceFromGround();
	bool bGrounded = SpringyLanding(GroundDist, DeltaTime);
	bGrounded |= Stand(GroundDist, DeltaTime);
	Walk(DeltaTime);
	
	// visuals
	bGrounded |= SquashAndStretch(GroundDist, DeltaTime);
	LookAround(GroundDist, DeltaTime);
	UpdateExcitement(DeltaTime);
	UpdateLimbs(DeltaTime);
	
	GroundedTime = bGrounded ? GroundedTime + DeltaTime : 0;
}

double ASootSprite::GetDistanceFromGround() const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(ASootSprite_GetDistanceFromGround)
	constexpr double TR_DOWN_DIST = 5000.0;
	FHitResult GroundHit;
	const FVector TrBegin = GetActorLocation();
	const FVector TrDir = FVector::DownVector;
	const FVector TrEnd = GetActorLocation() + TrDir * TR_DOWN_DIST;
	if (!GetWorld()->LineTraceSingleByChannel(GroundHit, TrBegin, TrEnd, GameTraceChannel::Ground))
	{
		return TR_DOWN_DIST * 2;
	}
#if UE_BUILD_DEBUG | UE_BUILD_DEVELOPMENT
	if (GAME_CONFIG()->Debug.bDrawSootSpriteGroundRaycast)
	{
		CreatureDebugDraw::HitResult(GetWorld(), GroundHit, -1);
	}
#endif
	return GroundHit.Distance;
}

double ASootSprite::GetVisualRadius() const
{
	return BodyMesh->Bounds.SphereRadius * 1.2;
}

float ASootSprite::IdealHeightFromGround(bool bFactorCarryingMass) const
{
	double IdealHeight = IDEAL_DISTANCE_FROM_GROUND * Settings.SizeScale;
	if (bFactorCarryingMass)
	{
		const double CarryMass = GetCarryingMass();
		const double MassRatio = FMath::Clamp(Settings.Mass * 10 / FMath::Sqrt(CarryMass), 0.1, 1);
		IdealHeight *= MassRatio;
	}
	return IdealHeight;
}

double ASootSprite::GetCarryingMass() const
{
	return CarryingMass + GAME_CONFIG()->Debug.SootSpriteAdditionalCarryingMass;
}

void ASootSprite::OnVisionBoxOverlap(UPrimitiveComponent* ThisComponent, AActor* OverlappedActor,
	UPrimitiveComponent* OverlappedComponent, int32 OtherBody, bool bFromSweep, const FHitResult& SweepResult)
{
	// simplicity: immediate roll for looktarget change
	if (FMath::FRand() < Excitement * 0.5)
	{
		LookTarget = OverlappedActor;
	}
}

bool ASootSprite::SpringyLanding(double GroundDist, float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(ASootSprite_FallAndCollide)
	
	const double SpringDisplacement = FMath::Min(GroundDist - GetVisualRadius(), 0);
	
	bool bGrounded = false;
	
	if (SpringDisplacement < 0)
	{
		bGrounded = true;
		
		const double SpringAccel = CreatureMath::CalculateSpringAcceleration(
			Settings.LandingSpringStiffness, Settings.LandingSpringDamping, SpringDisplacement, BodyMesh->GetPhysicsLinearVelocity().Z, Settings.Mass
		);
		
		const double AngularAlpha = FMath::GetMappedRangeValueClamped(
			FVector2D(0.25, 0.8), FVector2D(0, 0.7), -SpringDisplacement / GetVisualRadius()
		);
		
		const double SpringLinearAccel = SpringAccel * (1 - AngularAlpha);
		
		constexpr double BRAKING_ACCEL = 2000.0;
		constexpr double UPWARD_ACCEL  = 1000.0;
	
		const double RestitutionAccel = UPWARD_ACCEL * (-SpringDisplacement / GetVisualRadius());
		BodyMesh->AddForce(FVector(0,0, SpringLinearAccel + RestitutionAccel), NAME_None, true);
	
		const FVector LinearVelocity = BodyMesh->GetPhysicsLinearVelocity();
		if (LinearVelocity.Z < 0)
		{
			const double Decel = FMath::Min(BRAKING_ACCEL, -LinearVelocity.Z / DeltaTime);
			BodyMesh->AddForce(FVector(0,0, Decel), NAME_None, true);
		}
		else if (LinearVelocity.Z > 0)
		{
			const float Decel = CreatureMath::CalculateDeceleration(-SpringDisplacement, LinearVelocity.Z, BRAKING_ACCEL);
			BodyMesh->AddForce(FVector(0,0, -Decel), NAME_None, true);
		}
	}
	
	return bGrounded;
}

void ASootSprite::LookAround(double GroundDist, float DeltaTime)
{
	const double FallSpeed = FMath::Max(-BodyMesh->GetPhysicsLinearVelocity().Z, 0);
	Excitement = Settings.ExcitementFloor; // todo: more stuff
	if (GroundDist > 500 || FallSpeed > 500)
	{
		LookAt(GetActorLocation() + FVector::DownVector * 500, DeltaTime);
		Excitement = FMath::Clamp( FallSpeed / 1000, Settings.ExcitementFloor, 1.0);
	}
	else
	{
		if (!IsValid(LookTarget))
		{
			LookTarget = nullptr;
		}
		else
		{
			const FVector TowardTarget = LookTarget->GetActorLocation() - GetActorLocation();
			const bool bTargetBehind = (TowardTarget | GetActorForwardVector()) < 0;
			if (bTargetBehind)
			{
				LookTarget = nullptr;
			}
			else
			{
				LookAt(LookTarget->GetActorLocation(), DeltaTime);
			}
		}
		
		if (!LookTarget)
		{
			LookAt(GetActorLocation() + GetActorForwardVector() * 100 + FVector::DownVector * 80, DeltaTime);
		}
	}
}

void ASootSprite::LookAt(const FVector& Location, float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(ASootSprite_MovePupilsToward)
	const FVector WorldUp = FVector::UpVector;
	const FVector EyesForward = EyesMesh->GetComponentRotation().Vector();
	const FVector EyesRight = EyesForward.Cross(WorldUp);
	const FVector TowardTarget = (Location - GetActorLocation()).GetSafeNormal();
	
	// get the angle difference in radians between forward and ->target on the <x, y> plane
	
	const FVector2D EyesForwardXY = FVector2D(EyesForward).GetSafeNormal();
	const FVector2D TowardTargetXY = FVector2D(TowardTarget).GetSafeNormal();
	const double CosThetaXY = EyesForwardXY | TowardTargetXY;
	const double CosPhiXY = TowardTargetXY | FVector2D(-EyesRight);
	
	const double ThetaXY = FMath::Acos(CosThetaXY) * FMath::Sign(CosPhiXY);
	
	// get the angle difference in radians between forward and ->target on the <fore, z> plane
	
	const FVector2D EyesForwardZ = FVector2D(EyesForwardXY.Length(), EyesForward.Z).GetSafeNormal();
	const FVector2D TowardTargetZ = FVector2D(TowardTargetXY.Length(), TowardTarget.Z).GetSafeNormal();
	const double CosThetaZ = EyesForwardZ | TowardTargetZ;
	const double CosPhiZ = TowardTargetZ | FVector2D(0, 1);
	
	const double ThetaZ = FMath::Acos(CosThetaZ) * FMath::Sign(CosPhiZ);
	
	// rotate eyes mesh (/ turn head)
	
	FRotator EyesRotation = EyesMesh->GetRelativeRotation();
	
	// pitch
	
	constexpr double PITCH_MAX = 20.0;
	
	const double ThetaZDegrees = ThetaZ * (180/PI);
	const double ClampedTargetPitch = FMath::Clamp(EyesRotation.Pitch + ThetaZDegrees, -PITCH_MAX, PITCH_MAX);
	const double PitchDistance = ClampedTargetPitch - EyesRotation.Pitch;
	
	if (PitchDistance > 0.5f)
	{
		CreatureMath::AccelerateAndDecelerate(EyeSwivelVelocity.Pitch, PitchDistance, GetEyesSwivelAcceleration(), GetEyesSwivelDeceleration(), GetEyesSwivelMaxSpeed(), DeltaTime);
	}
	else
	{
		EyeSwivelVelocity.Pitch = 0;
	}
	
	// yaw
	
	constexpr double YAW_MAX = 80.0;
	
	const double ClampedYaw = FMath::Clamp(EyesRotation.Yaw, -YAW_MAX, YAW_MAX);
	const double MaxDist = FMath::Abs(YAW_MAX * FMath::Sign(ThetaXY) - ClampedYaw);
	const double ThetaXYDegrees = ThetaXY * (180/PI);
	const double YawDistance = FMath::Min(FMath::Abs(ThetaXYDegrees), MaxDist);
	
	if (YawDistance > 0.5f) 
	{
		CreatureMath::AccelerateAndDecelerate(EyeSwivelVelocity.Yaw, YawDistance * FMath::Sign(ThetaXY), GetEyesSwivelAcceleration(), GetEyesSwivelDeceleration(), GetEyesSwivelMaxSpeed(), DeltaTime);
	}
	else
	{
		EyeSwivelVelocity.Yaw = 0;
	}
	
	EyesRotation += EyeSwivelVelocity * DeltaTime;
	EyesRotation.Yaw = FMath::Clamp(EyesRotation.Yaw, -YAW_MAX, YAW_MAX);
	EyesRotation.Pitch = FMath::Clamp(EyesRotation.Pitch, -PITCH_MAX, PITCH_MAX);
	EyesRotation.Normalize();
	EyesMesh->SetRelativeRotation(EyesRotation);
	
	// pupils
	
	constexpr double PUPIL_POSITION_MAX = 0.98;
	
	// pupils track better vertically if you multiply scale up, I believe because 
	// eyes are typically squatty, but no scale = treat as if they were circles.
	// todo: base this on auto-measurements from the eyes in the texture.
	const double PupilThetaZ = FMath::Clamp(ThetaZ * 2.5, -1, 1);
	
	// take the radian difference from forward on each plane...
	// ... divide by pi  -> [0, 1] where 0 is forward and 1 is backward
	// ... multiply by 2 -> [0, 2] where 0 is forward, 1 is along the plane facing forward, and 2 is backward
	// basically, pupils can't look backward, and a decent way to encode that is to say that pupil space is in [0,1]
	// within the [0,2] superspace
	const double NormX = ThetaXY * (2.0 / PI);
	const double NormY = PupilThetaZ * (2.0 / PI);
	
	// clamp to what looks good
	const FVector2D NormPupilPos = {
		FMath::Clamp(NormX, -PUPIL_POSITION_MAX, PUPIL_POSITION_MAX),
		FMath::Clamp(NormY, -PUPIL_POSITION_MAX, PUPIL_POSITION_MAX),
	};
	
	MovePupilsToward(NormPupilPos, Settings.PupilMoveSpeed, DeltaTime);
}

void ASootSprite::MovePupilsToward(const FVector2D NormPosition, float LinearSpeed, float DeltaTime)
{
	PupilPosition = CreatureMath::VInterpConstantTo(PupilPosition, NormPosition, DeltaTime, LinearSpeed);
	GetEyesMaterial()->SetVectorParameterValue(L"PupilPosition", FVector(PupilPosition.X, PupilPosition.Y, 0));
}

bool ASootSprite::SquashAndStretch(double DistanceFromGround, float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(ASootSprite_SquashAndStretch)
	const double Radius = GetVisualRadius();
	const double MaxStretch = Settings.MaxStretch;
	double HzScaleTarget = 1.0;
	
	bool bGrounded = false;
	
	if (DistanceFromGround < Radius)
	{
		HzScaleTarget = FMath::GetMappedRangeValueClamped(
			FVector2D(1 - MaxStretch, 1), 
			FVector2D(1 + MaxStretch * 0.5, 1), 
			DistanceFromGround / Radius
		);
		bGrounded = true;
	}
	else
	{
		const FVector Up = GetActorUpVector();
		const double UpSpeed = FMath::Abs(BodyMesh->GetPhysicsLinearVelocity() | Up);
		
		if (UpSpeed > Settings.StretchSpeedMin)
		{
			const double Range = Settings.StretchSpeedMax - Settings.StretchSpeedMin;
			const double UpSpeedNorm = (UpSpeed - Settings.StretchSpeedMin) / Range;
			const double VtScaleTarget = FMath::GetMappedRangeValueClamped(
				FVector2D(0, 1),
				FVector2D(1, 1 + MaxStretch * 0.5),
				UpSpeedNorm
			);
			HzScaleTarget = FMath::Abs(2 - VtScaleTarget);
		}
	}
	
	double HzScale = Old_BodyScale.X;
	const double NormScaleDelta = FMath::Clamp(DeltaTime * Settings.StretchScaleSpeed, 0, 1);
	double InterpolatedScale = HzScale + (HzScaleTarget - HzScale) * NormScaleDelta; 
	
	SetBodyScale({InterpolatedScale, FMath::Abs(2 - InterpolatedScale)});
	
	return bGrounded;
}

void ASootSprite::SetBodyScale(const FVector2D& Scale)
{
	constexpr double ERROR = 1e-3;
	if (FMath::Abs(Old_BodyScale.X - Scale.X) > ERROR || FMath::Abs(Old_BodyScale.Y - Scale.Y) > ERROR)
	{
		EyesMesh->SetWorldScale3D(Settings.SizeScale * FVector(Scale.X, Scale.X, Scale.Y + Excitement));
		const double SquishAlpha = FMath::GetMappedRangeValueClamped(FVector2D(1.05, 1.2), FVector2D(0, 1), Scale.X / Scale.Y);
		GetEyesMaterial()->SetScalarParameterValue(L"Squish", SquishAlpha);
		SetBodySpriteScale(Settings.SizeScale * Scale);
		Old_BodyScale = Scale;
	}
}

void ASootSprite::SetBodySpriteScale(const FVector2D& Scale) const
{
	BodySprite->Elements[0].BaseSizeX = BaseSpriteSize * Scale.Y;
	BodySprite->Elements[0].BaseSizeY = BaseSpriteSize * Scale.X;
	BodySprite->MarkRenderStateDirty();
}

bool ASootSprite::Stand(double GroundDist, float DeltaTime)
{
	bool bGrounded = false;
	const bool bWasStanding = bStanding;
	bStanding = false;
	
	if (GroundedTime > 0.5)
	{
		const double AverageFootZ = (GetLimb(LeftLeg)->GetTarget() + GetLimb(RightLeg)->GetTarget()).Z * 0.5;
		const double StandSpringHeight = AverageFootZ + IdealHeightFromGround();
		const double SpringDisplacement = FMath::Min(GetActorLocation().Z - StandSpringHeight, 0);
		const FVector LinearVelocity = BodyMesh->GetPhysicsLinearVelocity();
	
		const double SpringAccel = CreatureMath::CalculateSpringAcceleration(
			Settings.LimbSpringStiffness, Settings.LimbSpringDamping, SpringDisplacement, LinearVelocity.Z, Settings.Mass
		);
		BodyMesh->AddForce(FVector(0,0,SpringAccel), NAME_None, true);
		
		// disabling auto-sit for better video result
		// StandingAirTime = SpringDisplacement < 200 ? 0 : StandingAirTime + DeltaTime;
		
		if (StandingAirTime < 0.5)
		{
			if (!bWasStanding)
			{
				BOTH_LEGS_DO(SetTargetDirection(FVector::DownVector))
				BOTH_LEGS_DO(TargetInterpBegin = CurrentLimb->GetTarget())
				BOTH_LEGS_DO(TargetInterpEnd = CurrentLimb->GetTarget())
				for (USootSpriteLimb* Toe : Toes)
				{
					Toe->TargetLength = GetToeLength();
				}
			}
			
			// todo: state
			if (GroundedTime < 1)
			{
				BOTH_LEGS_DO(TargetLength = (StandSpringHeight - GetVisualRadius() * 0.5 + SpringDisplacement))
			}
			
			bGrounded = true;
			bStanding = true;
			
			ManageCenterOfMass(DeltaTime);
		}
	}
	
	if (!bStanding)
	{
		BOTH_LEGS_DO(TargetLength = 0)
		for (USootSpriteLimb* Toe : Toes)
		{
			Toe->TargetLength = 0;
		}
	}

	return bGrounded;
}

void ASootSprite::Walk(float DeltaTime)
{
	if (bStanding && GroundedTime > 1)
	{
		WalkPhase += (Settings.WalkCycleRate * (1 + Excitement)) * DeltaTime;
		
		bool bInitializedLimb[4] = {false};
		USootSpriteLimb* PrimaryLeg = Limbs[MovingLeg];
		
		while (WalkPhase >= 1)
		{
			WalkPhase -= 1;
			
			const FVector Forward2D3 = GetActorForwardVector().GetSafeNormal2D();
			const FVector2D Forward2D (Forward2D3.X, Forward2D3.Y);
			
			MovingLeg = (ESootSpriteLimbType)!MovingLeg;
			PrimaryLeg = Limbs[MovingLeg];
			
			if (!bInitializedLimb[MovingLeg])
			{
				const FSootSpriteLimbResult Result = PrimaryLeg->InitializeLegTargetInterpolation(Forward2D * 250, 500, 200, GAME_CONFIG()->Debug.bDrawSootSpriteFootHeightRaycast);
				bInitializedLimb[MovingLeg] = true;
				
				// wall / ledge. for video, I know it's the ledge. 
				// give up walking and fall off the ledge
				if (Result.InitializeLegTargetInterpolation.Progress == 0)
				{
					bStanding = false;
					GroundedTime = 0;
					BodyMesh->AddForce(GetActorForwardVector() * 5000, NAME_None, true);
					break;
				}
			}
		}
		
		if (bStanding)
		{
			USootSpriteLimb* OtherLeg = Limbs[(ESootSpriteLimbType)!MovingLeg];
		
			const double CurZOffset = GetDistanceFromGround(); // todo: replace
			const double NextZOffset = FMath::Abs(GetActorLocation().Z - PrimaryLeg->TargetInterpEnd.Z);
			double ZDiff = FMath::Abs(CurZOffset - NextZOffset);
		
			PrimaryLeg->TargetLength = (PrimaryLeg->GetTarget() - PrimaryLeg->GetOrigin()).Size() * 1.1 + ZDiff;
			OtherLeg->TargetLength = (OtherLeg->GetTarget() - OtherLeg->GetOrigin()).Size() * 1.1;
		
			if (MovingLeg == LeftLeg)
			{
				LegCOMWeight = WalkPhase;
			}
			else
			{
				LegCOMWeight = 1 - WalkPhase;
			}
		
			constexpr double MIDDLE_WEIGHT = 0.25;
			LegCOMWeight = (LegCOMWeight * (1-MIDDLE_WEIGHT)) + (0.5 * MIDDLE_WEIGHT);
		
			PrimaryLeg->InterpolateTarget(WalkPhase);
		}
	}
	else
	{
		// setting up a reset next time we start walking
		WalkPhase = 1;
	}
	
	GetLimb(RightLeg)->CowboyDir = GetActorRightVector();
	GetLimb(LeftLeg)->CowboyDir = -GetLimb(RightLeg)->CowboyDir;
}

void ASootSprite::ManageCenterOfMass(float DeltaTime)
{
	const FVector COM = GetLimb(LeftLeg)->GetTarget() * LegCOMWeight + GetLimb(RightLeg)->GetTarget() * (1-LegCOMWeight);
	
	const FVector Velocity = BodyMesh->GetPhysicsLinearVelocity();
	const FVector ComDiff = (COM - GetActorLocation());

	const FVector ForeDir = GetActorForwardVector().GetSafeNormal2D();
	const FVector RightDir = GetActorRightVector().GetSafeNormal2D();
	
	const double ComDistFore = ComDiff | ForeDir;
	const double ComDistRight = ComDiff | RightDir;
	
	const double SpeedFore = Velocity | ForeDir;
	const double SpeedRight = Velocity | RightDir;
	
	const double ForeSpringAccel = CreatureMath::CalculateSpringAcceleration(
		200, 0.99, -ComDistFore, SpeedFore, Settings.Mass
	);
	
	const double SideSpringAccel = CreatureMath::CalculateSpringAcceleration(
		200, 0.99, -ComDistRight, SpeedRight, Settings.Mass
	);
	
	BodyMesh->AddForce(ForeDir * ForeSpringAccel + RightDir * SideSpringAccel, NAME_None, true);
}

void ASootSprite::ApplyLimbSpring(USootSpriteLimb* Limb, double Weight)
{
	const FVector LimbDiff = Limb->GetTarget() - Limb->GetOrigin();
	const FVector LimbDiff2D(LimbDiff.X, LimbDiff.Y, 0);
	const double LimbDistanceSq = LimbDiff2D.SizeSquared();
	if (LimbDistanceSq < 1e-6)
	{
		return;
	}
	const double LimbDistance2D = FMath::Sqrt(LimbDistanceSq);
	const FVector LimbDir = -LimbDiff2D / LimbDistance2D;
	const double SpringDisplacement = LimbDistance2D;
	const float SpeedAlongSpring = BodyMesh->GetPhysicsLinearVelocity() | LimbDir;
	const double SpringAccel = CreatureMath::CalculateSpringAcceleration(
		100, 1.0, SpringDisplacement, SpeedAlongSpring, Settings.Mass
	);
	BodyMesh->AddForce(LimbDir * SpringAccel * Weight, NAME_None, true);
}

void ASootSprite::UpdateLimbs(float DeltaTime)
{
	// this should *NOT* be this way. the feet/hands should be part of the limb mesh.
	// but, I'm on a deadline to finish this enough to show it and I wasn't thinking about toesies originally, so individual toes it is.
	
	// set where toes begin
	
	Toes[LeftFoot_LeftToe]->AttachOrigin.Offset = GetLimb(LeftLeg)->GetTarget();
	Toes[LeftFoot_MiddleToe]->AttachOrigin.Offset = GetLimb(LeftLeg)->GetTarget();
	Toes[LeftFoot_RightToe]->AttachOrigin.Offset = GetLimb(LeftLeg)->GetTarget();
	
	Toes[RightFoot_LeftToe]->AttachOrigin.Offset = GetLimb(RightLeg)->GetTarget();
	Toes[RightFoot_MiddleToe]->AttachOrigin.Offset = GetLimb(RightLeg)->GetTarget();
	Toes[RightFoot_RightToe]->AttachOrigin.Offset = GetLimb(RightLeg)->GetTarget();
	
	const FVector Forward = GetActorForwardVector();
	const FVector Right =  GetActorRightVector();
	
	// little hacky. 
	// toe length < soot sprite radius. this just needs to be longer than the toe length.
	const double ForeDist = GetVisualRadius();
	
	const FVector ForeOffset = Forward * ForeDist;
	const FVector RightOffset = Right * (ForeDist * 0.75);
	const FVector LeftOffset = -Right * (ForeDist * 0.75);
	
	// set where toes end
	
	Toes[LeftFoot_LeftToe]->AttachTarget.Offset = Toes[LeftFoot_LeftToe]->AttachOrigin.Offset + ForeOffset + LeftOffset;
	Toes[LeftFoot_MiddleToe]->AttachTarget.Offset = Toes[LeftFoot_MiddleToe]->AttachOrigin.Offset + ForeOffset;
	Toes[LeftFoot_RightToe]->AttachTarget.Offset = Toes[LeftFoot_RightToe]->AttachOrigin.Offset + ForeOffset + RightOffset;
	
	Toes[RightFoot_LeftToe]->AttachTarget.Offset = Toes[RightFoot_LeftToe]->AttachOrigin.Offset + ForeOffset + LeftOffset;
	Toes[RightFoot_MiddleToe]->AttachTarget.Offset = Toes[RightFoot_MiddleToe]->AttachOrigin.Offset + ForeOffset;
	Toes[RightFoot_RightToe]->AttachTarget.Offset = Toes[RightFoot_RightToe]->AttachOrigin.Offset + ForeOffset + RightOffset;
	
	// per-limb updates
	
	for (USootSpriteLimb* Limb : Limbs)
	{
		Limb->TickUpdate(DeltaTime);
	}
	
	for (USootSpriteLimb* Toe : Toes)
	{	
		Toe->TickUpdate(DeltaTime);
	}
}

void ASootSprite::UpdateExcitement(float DeltaTime)
{
	if (Excitement != PrevExcitement)
	{
		// do exciting things!
	}
	PrevExcitement = Excitement;
}

void ASootSprite::Debug_AddVelocity(const FVector& DeltaVelocity)
{
#if UE_BUILD_DEBUG | UE_BUILD_DEVELOPMENT
	BodyMesh->AddForce(DeltaVelocity / GetWorld()->DeltaTimeSeconds, NAME_None, true);
#endif
}

void ASootSprite::Debug_DrawBounds() const
{
	const FBox BoxBounds = EyesMesh->Bounds.GetBox();
	const float VtRadius = GetVisualRadius();
	const float HzRadius = GetVisualRadius();
	
	// total bounds
	DrawDebugBox(GetWorld(), BoxBounds.GetCenter(), BoxBounds.GetExtent(), FColor::Magenta);
	
	const FVector VtExtent = FVector(10,10,VtRadius);
	const FBox VtBox (GetActorLocation()-VtExtent, GetActorLocation()+VtExtent);
	
	const FVector HzExtent = FVector(HzRadius,HzRadius,10);
	const FBox HzBox (GetActorLocation()-HzExtent, GetActorLocation()+HzExtent);
	
	DrawDebugBox(GetWorld(), VtBox.GetCenter(), VtBox.GetExtent(), FColor::Blue);
	DrawDebugBox(GetWorld(), HzBox.GetCenter(), HzBox.GetExtent(), FColor::Red);
}

void ASootSprite::Debug_DrawVisionBox() const
{
	const FVector BoxCenter = VisionBox->GetComponentLocation();
	const FVector Extent = VisionBox->GetScaledBoxExtent();
	DrawDebugBox(GetWorld(), BoxCenter, Extent, FColor::Orange, false, -1, 0, 3);
}
