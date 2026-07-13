#pragma once

#include "Components/InstancedStaticMeshComponent.h"
#include "SootSpriteLimb.generated.h"

enum class ELimbAttachTargetType
{
	Actor,
	World
};

USTRUCT()
struct FSootSpriteLimbAttachment
{
	GENERATED_BODY()
	UPROPERTY()
	AActor* Actor = nullptr;
	FVector Offset = FVector(0);
	ELimbAttachTargetType Type;
};

enum class ESootSpriteLimbGrowState : uint8
{
	None,
	Grow,
	Retract
};

struct FSootSpriteLimbResult
{
	enum EMessage : uint32
	{
		None = 0,
		InvalidInput,
		RaycastDidNotHit,
		RaycastHitOverMaxStepHeight,
		RaycastHitOverMaxEarlyOutStepHeight
	};
	
	EMessage Message = None;
	
	union
	{
		struct FInitializeLegTargetInterpolation
		{
			float Progress = 0;
		} InitializeLegTargetInterpolation;
	};
};


UCLASS()
class CREATUREPROGRAMMER_API USootSpriteLimb : public UActorComponent
{
	GENERATED_BODY()
	
	friend class USootSpriteLimbISM;
	friend class ASootSprite;
	
public:

	void TickUpdate(float DeltaTime);
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	void Clear();
	
	void SetGrowState(ESootSpriteLimbGrowState InGrowState);
	
	virtual void BeginPlay() override;
	
	static FVector GetBindJointPosition(int32 i);
	
	FVector GetOrigin() const;
	
	const FVector& GetTarget() const;
	
	ASootSprite* GetSootSprite() const;
	
	void SetTargetDirection(const FVector& Direction, double NormalOffset=0);
	
	// returns 0-to-1 alpha representing the normalized 'how far we were able to step' when moving toward the anchor.
	// 0 = no progress, 1 = made it, 0 < x < 1 = ran into something impassable
	FSootSpriteLimbResult InitializeLegTargetInterpolation(const FVector2D& AnchorOffset, double MaxStepHeight, double EarlyOutStepHeight, bool bDebugDraw=false);
	
	void InterpolateTarget(float Alpha);
	
	double GetLength() const;
	
protected:
	
	void Grow(float DelaTime);
	
	void ResetJointPoses();
	
	void DetermineJointRotations();
	
	void PackSkinningMatrices();
	
	void SetJointPosition(int32 Joint, const FVector& WorldPosition);
	
	void Bend();
	
	FVector FindBendDirection(int32& OutStepCount, const FVector& Origin, const FVector& Dir, const FVector& RotationAxis, double Dist, double ClearanceAngle=30.0);
	
public:
	
	static constexpr int32 JOINT_COUNT = 5;
	static constexpr int32 PARAMETERS_PER_JOINT = 12;
	static constexpr int32 JOINT_PARAMETER_COUNT = JOINT_COUNT * PARAMETERS_PER_JOINT;
	static constexpr float MESH_HALF_WIDTH = 4.0f;
	static constexpr float MESH_LENGTH = 100.f;
	static constexpr double LEG_LENGTH_MULTIPLIER_FOR_BEND = 1.0025;
	
protected:
	
	UPROPERTY()
	FSootSpriteLimbAttachment AttachOrigin;
	
	UPROPERTY()
	FSootSpriteLimbAttachment AttachTarget;
	
	FVector TargetInterpBegin;
	FVector TargetInterpEnd;
	
	int32 MeshInstance = -1;
	
	TStaticArray<FTransform, JOINT_COUNT> JointPoses;
	TArray<float, TInlineAllocator<JOINT_PARAMETER_COUNT>> PackedSkinningMatrices;
	
	double Length = 1;
	double TargetLength = 1;
	
	FVector2D Hack_LastAnchorOffset = FVector2D::ZeroVector;
	FVector BendDir = FVector::ForwardVector;
	FVector CowboyDir = FVector::ForwardVector;
	
	ESootSpriteLimbGrowState GrowState = ESootSpriteLimbGrowState::None;
	
	uint8 bEndNodeAnchored : 1 = false;
};

UCLASS()
class CREATUREPROGRAMMER_API USootSpriteLimbISM : public UInstancedStaticMeshComponent
{
	GENERATED_BODY()
	
public:
	
	USootSpriteLimbISM();
	
	static USootSpriteLimbISM* Get(const UWorld* World);
	
	void Init();
	
	// build the limb mesh and assign it to this component (used at runtime)
	void BuildMesh();
	
	void TickUpdate(float DeltaTime);
	
	void RequestInstance(USootSpriteLimb* Limb);
	
	void ReturnInstance(USootSpriteLimb* Limb);
	
	FMatrix& GetInstanceTransformMatrix(USootSpriteLimb* Limb);
	
	void UpdateInstance(USootSpriteLimb* Limb);
	
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	
public:
	
	static constexpr int32 SEGMENT_COUNT = 32;
	
protected:
	
	UPROPERTY()
	UStaticMesh* Mesh;
	
	UPROPERTY()
	TArray<USootSpriteLimb*> TrackedLimbs;
	
	bool bInitialized;
	
public:
	
	// debug
	
	void Debug_DrawBounds() const;
	
	void Debug_DrawLimbMeshBounds() const;
};
