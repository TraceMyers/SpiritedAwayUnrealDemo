#pragma once

#include "CoreMinimal.h"
#include "SootSpriteAI.h"
#include "SootSpriteLimb.h"
#include "Components/MaterialBillboardComponent.h"
#include "CreatureProgrammer/Config/SootSpriteSettings.h"
#include "CreatureProgrammer/Creatures/Creature.h"
#include "CreatureProgrammer/Helpers/MacroHelpers.h"
#include "CreatureProgrammer/Helpers/CreatureMath.h"
#include "Sootsprite.generated.h"

class UStaticMeshComponent;
class UMaterialBillboardComponent;
class UBoxComponent;

enum class ESootSpriteMoveState : uint8
{
	Scoot,
	Walk,
	TransitionToWalk,
	TransitionToScoot
};

// soot sprite creature class. little sooty fuzzball with three-toed feet
// and bendy legs from that one scene in Sprited Away.
UCLASS()
class CREATUREPROGRAMMER_API ASootSprite : public ACreature
{
	GENERATED_BODY()
	
	friend class USootSpriteLimb;
	
	enum ESootSpriteLimbType : uint8
	{
		LeftLeg = 0,
		RightLeg = 1,
		LeftArm,
		RightArm
	};
	
	// this is silly but prototypes can be like that
	enum ESootSpriteToe : uint8
	{
		LeftFoot_LeftToe,
		LeftFoot_MiddleToe,
		LeftFoot_RightToe,
		RightFoot_LeftToe,
		RightFoot_MiddleToe,
		RightFoot_RightToe,
	};
	
public:	
	
	ASootSprite();
	
	void TickUpdate(float DeltaTime);
	
	double GetDistanceFromGround() const;
	
	double GetVisualRadius() const;
	
	double GetToeLength() const { return GetVisualRadius() * 0.38; }
	
	float IdealHeightFromGround(bool bFactorCarryingMass=false) const;
	
	double GetCarryingMass() const;
	
protected:
	
	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	// when landing from a fall while legs retracted, treat outer radius as a soft springy material before reaching the inner rigidbody collision 
	bool SpringyLanding(double GroundDist, float DeltaTime);
	
	void LookAround(double GroundDist, float DeltaTime);
	
	void LookAt(const FVector& Location, float DeltaTime);
	
	// todo: movement curve, remembering start position, etc.
	void MovePupilsToward(const FVector2D NormPosition, float LinearSpeed, float DeltaTime);
	
	bool SquashAndStretch(double DistanceFromGround, float DeltaTime);
	
	// X -> XY, Y -> Z
	void SetBodyScale(const FVector2D& Scale);
	
	void SetBodySpriteScale(const FVector2D& Scale) const;
	
	// track whether we should or shouldn't be standing.
	// apply forces to push us up off the ground, as legs would.
	bool Stand(double DistanceFromGround, float DeltaTime);
	
	void UpdateLegLengthsWhileWalking();
	
	void UpdateCenterOfMassWhileWalking();
	
	void Walk(float DeltaTime);
	
	void ManageCenterOfMass(float DeltaTime);
	
	void ApplyLimbSpring(USootSpriteLimb* Limb, double Weight);
	
	void UpdateLimbs(float DeltaTime);
	
	FORCEINLINE USootSpriteLimb* GetLimb(ESootSpriteLimbType LimbType)
	{
		return Limbs[LimbType];
	}
	
	FORCEINLINE USootSpriteLimb* GetToe(ESootSpriteToe Toe)
	{
		return Toes[Toe];
	}
	
	DEFINE_DYNAMIC_MATERIAL_INSTANCE_GETTER(GetEyesMaterial, EyesMesh, 0)
	DEFINE_DYNAMIC_MATERIAL_INSTANCE_GETTER(GetBodySpriteMaterial, BodySprite, 0);

	DEFINE_EXCITEMENT_SCALED_GETTER(EyesSwivelAcceleration, EyesSwivelExcitementMultiplierMax)
	DEFINE_EXCITEMENT_SCALED_GETTER(EyesSwivelDeceleration, EyesSwivelExcitementMultiplierMax)
	DEFINE_EXCITEMENT_SCALED_GETTER(EyesSwivelMaxSpeed, EyesSwivelExcitementMultiplierMax)
	
	void UpdateExcitement(float DeltaTime);
	
public:
	
	static constexpr double IDEAL_DISTANCE_FROM_GROUND = 180.0;
	
	UPROPERTY(VisibleDefaultsOnly)
	UStaticMeshComponent* BodyMesh;
	
	UPROPERTY(VisibleDefaultsOnly)
	UStaticMeshComponent* EyesMesh;
	
	UPROPERTY(VisibleDefaultsOnly)
	UMaterialBillboardComponent* BodySprite;
	
	UPROPERTY(EditInstanceOnly)
	AActor* LookTarget;
	
	UPROPERTY()
	AActor* DesireTarget;
	
	UPROPERTY()
	AActor* CarryingActor;
	
	UPROPERTY()
	TArray<USootSpriteLimb*> Limbs;
	
	// toes should not be individual mesh instances... but it was the fastest way and I'm out of time for making this video
	UPROPERTY()
	TArray<USootSpriteLimb*> Toes;
	
	UPROPERTY()
	FSootSpriteSettings Settings;
	
	UPROPERTY(EditInstanceOnly)
	bool bDisableUpdate = false;
	
	static constexpr double RADIUS_VISUAL_ADJUSTMENT_SCALE = 1.2;
	
protected:
	
	// protected fields
	
	static double BaseSpriteSize;
	static FAlphaSampler WalkLimbHeightSampler;
	
	FVector2D PupilPosition;
	FVector2D Old_BodyScale = {1, 1};
	
	// FVector LinearVelocity;
	FRotator AngularVelocity;
	FRotator EyeSwivelVelocity;
	
	FSootSpriteAI AI;
	
	ESootSpriteMoveState MoveState = ESootSpriteMoveState::TransitionToWalk;
	
	double GroundedTime = 0;
	double StandingAirTime = 0;
	
	double WalkPhase = 0.5;
	double AverageSpeed = 0;
	double TargetSpeed = 0;
	
	double Excitement = 0;
	double PrevExcitement = 0;
	double LookTargetAversion = 0;
	
	double LegCOMWeight = 0.5;
	double CarryingMass = 0;
	
	ESootSpriteLimbType MovingLeg = LeftLeg;
	
	bool bStanding = false;
	
public:
	
	void Debug_AddVelocity(const FVector& DeltaVelocity);
	
	void Debug_DrawBounds() const;
	
	void Debug_DrawVisionBox() const;
};
