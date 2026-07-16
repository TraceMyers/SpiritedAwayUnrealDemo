#include "SootSpriteLimb.h"

#include "SootSprite.h"
#include "StaticMeshAttributes.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "CreatureProgrammer/GameBase/Goober.h"
#include "CreatureProgrammer/GameBase/GooberGameState.h"
#include "CreatureProgrammer/Helpers/CreatureMath.h"
#include "CreatureProgrammer/Helpers/DebugDraw.h"

USootSpriteLimbISM::USootSpriteLimbISM()
{
	Mobility = EComponentMobility::Movable;
}

USootSpriteLimbISM* USootSpriteLimbISM::Get(const UWorld* World)
{
	return CastChecked<AGooberGameState>(World->GetGameState())->Goober->SootSpriteLimbIsm;
}

void USootSpriteLimbISM::Init()
{
	BuildMesh();
	SetNumCustomDataFloats(USootSpriteLimb::JOINT_PARAMETER_COUNT);
	SetSimulatePhysics(false);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	bInitialized = true;
}

void USootSpriteLimbISM::BuildMesh()
{
	check(Mesh == nullptr)
	
    FMeshDescription MeshDesc;
    FStaticMeshAttributes Attrs(MeshDesc);
    Attrs.Register();

    // 3 UV channels: UV0 = normal texcoords, UV1 = bone indices, UV2.x = weight0
    TVertexInstanceAttributesRef<FVector2f> UVs = Attrs.GetVertexInstanceUVs();
    UVs.SetNumChannels(3);

    FPolygonGroupID PolyGroup = MeshDesc.CreatePolygonGroup();
    Attrs.GetPolygonGroupMaterialSlotNames()[PolyGroup] = FName("LimbMat");

    // --- geometry: 3 long quads crossed at 60°, NumSegments rings ---

	constexpr int32 PLANE_COUNT = 3;
	constexpr float JOINT_INFLUENCE_EXTENT = 1.5f;
	
	// bone influence
	// (b0)------------------------|
	// (b1)|--------------------------------------------|
	// (b2)      |---------------------------------------------------------|
	// (b3)                        |------------------------------------------------|
	// (b4)                                             |----------------------------|
	// b0        		 b1        			 b2        			 b3        			 b4
	// o-----------------o-------------------o-------------------o-------------------o
	
	FVector2f BoneInfluenceEdges[USootSpriteLimb::JOINT_COUNT] = {
		{-JOINT_INFLUENCE_EXTENT, JOINT_INFLUENCE_EXTENT},
		{-1, JOINT_INFLUENCE_EXTENT},
		{-JOINT_INFLUENCE_EXTENT, JOINT_INFLUENCE_EXTENT},
		{-JOINT_INFLUENCE_EXTENT, 1},
		{-JOINT_INFLUENCE_EXTENT, JOINT_INFLUENCE_EXTENT}
	};

    for (int32 Plane = 0; Plane < PLANE_COUNT; ++Plane)
    {
        const float Angle = Plane * PI / PLANE_COUNT;
        FVector3f HorizontalSide(FMath::Cos(Angle) * USootSpriteLimb::MESH_HALF_WIDTH, FMath::Sin(Angle) * USootSpriteLimb::MESH_HALF_WIDTH, 0);

    	// 2 verts per ring, NumSegments+1 rings
        TArray<FVertexInstanceID> Vertices; 
    	
        for (int32 Segment = 0; Segment <= SEGMENT_COUNT; ++Segment)
        {
        	// 0 root → 1 tip
            float NormHeight = (float)Segment / SEGMENT_COUNT;              
            FVector3f HeightOffset(0, 0, NormHeight * USootSpriteLimb::MESH_LENGTH);
        	
        	struct FIndexAndWeight
        	{
        		int32 Index;
        		float Weight;
        	};
        	
        	TArray<FIndexAndWeight, TInlineAllocator<USootSpriteLimb::JOINT_COUNT>> BoneInfluences;
        	BoneInfluences.SetNum(USootSpriteLimb::JOINT_COUNT);
        	
        	for (int32 b = 0; b < USootSpriteLimb::JOINT_COUNT; b++)
        	{
				const float BoneCenterDiff = NormHeight * 4 - (float)b;
        		BoneInfluences[b].Index = b;
        		if (BoneCenterDiff < 0)
        		{
        			BoneInfluences[b].Weight = FMath::GetMappedRangeValueClamped(
        				FVector2f(BoneInfluenceEdges[b].X, 0), 
        				FVector2f(0, 1),
        				BoneCenterDiff
					);
        		}
        		else
        		{
        			BoneInfluences[b].Weight = FMath::GetMappedRangeValueClamped(
						FVector2f(BoneInfluenceEdges[b].Y, 0), 
						FVector2f(0, 1),
						BoneCenterDiff
					);
        		}
        	}
        	
        	
        	BoneInfluences.Sort([](const FIndexAndWeight& A, const FIndexAndWeight& B)
        	{
        		return A.Weight > B.Weight;
        	});
        	
        	float WeightSum = 0;
        	constexpr int32 BONE_INFLUENCES_PER_VERTEX = 3;
        	for (int i = 0; i < BONE_INFLUENCES_PER_VERTEX; i++)
        	{
        		WeightSum += BoneInfluences[i].Weight;
        	}
        	
        	check (WeightSum > 0)
        	const float WeightNormFactor = 1 / WeightSum;
        	for (int i = 0; i < BONE_INFLUENCES_PER_VERTEX; i++)
        	{
        		BoneInfluences[i].Weight *= WeightNormFactor;
        	}

            for (int32 Side = 0; Side < 2; Side++)
            {
                FVertexID V = MeshDesc.CreateVertex();
                Attrs.GetVertexPositions()[V] = HeightOffset + (Side ? HorizontalSide : -HorizontalSide);
            	
				FVertexInstanceID I = MeshDesc.CreateVertexInstance(V);
            	UVs.Set(I, 0, FVector2f(BoneInfluences[0].Index, BoneInfluences[0].Weight));
            	UVs.Set(I, 1, FVector2f(BoneInfluences[1].Index, BoneInfluences[1].Weight));
            	UVs.Set(I, 2, FVector2f(BoneInfluences[2].Index, BoneInfluences[2].Weight));           	
            	
                Vertices.Add(I);
            }
        	
            // build quads once we have two rings
            if (Segment > 0)
            {
                int32 PrevOffset = Vertices.Num() - 4;
            	
                FVertexInstanceID Prev0 = Vertices[PrevOffset+0];
            	FVertexInstanceID Prev1 = Vertices[PrevOffset+1];
                FVertexInstanceID Cur0  = Vertices[PrevOffset+2];
            	FVertexInstanceID Cur1  = Vertices[PrevOffset+3];
                	
                MeshDesc.CreateTriangle(PolyGroup, {Prev0, Prev1, Cur0});
                MeshDesc.CreateTriangle(PolyGroup, {Prev1, Cur0, Cur1});
            }
        }
    }

    Mesh = NewObject<UStaticMesh>(this);
	
	Mesh->GetStaticMaterials().Add(
		FStaticMaterial(
			GAME_CONFIG()->SootSprite.LimbMaterial,
			FName("LimbMat"),
			FName("LimbMat")
		)
	);

    UStaticMesh::FBuildMeshDescriptionsParams Params;
    Params.bBuildSimpleCollision = false;
    Params.bFastBuild = true;               // required for runtime
	
    Mesh->BuildFromMeshDescriptions({&MeshDesc}, Params);
	Mesh->SetMaterial(0, GAME_CONFIG()->SootSprite.LimbMaterial);
	
	// animation moves outside of static bounds.
	Mesh->SetExtendedBounds(Mesh->GetExtendedBounds().ExpandBy(500));
	// I beleive sdf's are built from untransformed vertices. not useful.
	Mesh->bGenerateMeshDistanceField = false;
	
	SetStaticMesh(Mesh);
}

void USootSpriteLimbISM::TickUpdate(float DeltaTime)
{
	MarkRenderStateDirty();
}

void USootSpriteLimbISM::RequestInstance(USootSpriteLimb* Limb)
{
	check(bInitialized)
	check(TrackedLimbs.Num() == PerInstanceSMData.Num())
	Limb->MeshInstance = TrackedLimbs.Num();
	AddInstance(FTransform::Identity);
	
	// typically storing raw pointers like this is frowned upon. i do this quite a bit in personal projects where
	// the pointer storage is stable or well-accounted for. I understand if you still don't like it. it's just a fastish
	// solution and I need to get this demo into the world this week.
	TrackedLimbs.Add(Limb);
}

void USootSpriteLimbISM::ReturnInstance(USootSpriteLimb* Limb)
{
	check(TrackedLimbs.Num() == PerInstanceSMData.Num())
	
	// remove swap body instance
	RemoveInstanceInternal(Limb->MeshInstance, false, true, false);
	
	// remove swap slot
	TrackedLimbs[TrackedLimbs.Num()-1]->MeshInstance = Limb->MeshInstance;
	TrackedLimbs[Limb->MeshInstance] = TrackedLimbs[TrackedLimbs.Num()-1];
	TrackedLimbs.SetNum(TrackedLimbs.Num()-1);
	
	Limb->MeshInstance = -1;
}

FMatrix& USootSpriteLimbISM::GetInstanceTransformMatrix(USootSpriteLimb* Limb)
{
	return PerInstanceSMData[Limb->MeshInstance].Transform;
}

void USootSpriteLimbISM::UpdateInstance(USootSpriteLimb* Limb)
{
	PROFILE_FUNCTION()
	
	// todo: batch update
	
	check(Limb->MeshInstance >= 0)	
	TArrayView<float> JointFloatsView = TArrayView<float>(Limb->PackedSkinningMatrices).Slice(0, Limb->PackedSkinningMatrices.Num());
	SetCustomData(Limb->MeshInstance, JointFloatsView, true);
	
	const FVector LimbOrigin = Limb->GetOrigin();
	const FTransform InstanceTransform(
		FQuat::Identity,
		LimbOrigin,
		FVector::OneVector
	);
	LIMB_ISM()->UpdateInstanceTransform(Limb->MeshInstance, InstanceTransform, true, false, true);
}

FBoxSphereBounds USootSpriteLimbISM::CalcBounds(const FTransform& LocalToWorld) const
{
	if (PerInstanceSMData.Num() == 0)
	{
		return FBoxSphereBounds(FBox(FVector(-1), FVector(1)));
	}
	FBox Box(ForceInit);
	for (const FInstancedStaticMeshInstanceData& Instance : PerInstanceSMData)
	{
		const FVector Pos = Instance.Transform.TransformPosition({});
		Box += Pos;
	}
	constexpr double LIMB_PADDING = 800.0;
	return FBoxSphereBounds(Box.ExpandBy(LIMB_PADDING).TransformBy(LocalToWorld));
}

void USootSpriteLimbISM::Debug_DrawBounds() const
{
	FBox BoundsBox = Bounds.TransformBy(GetComponentTransform()).GetBox();
	DrawDebugBox(GetWorld(), BoundsBox.GetCenter(), BoundsBox.GetExtent(), FColor::Magenta, false, -1, 0, 5);
}

void USootSpriteLimbISM::Debug_DrawLimbMeshBounds() const
{
	FBox BoundsBox = Mesh->GetExtendedBounds().TransformBy(GetComponentTransform()).GetBox();
	DrawDebugBox(GetWorld(), BoundsBox.GetCenter(), BoundsBox.GetExtent(), FColor::Blue, false, -1, 0, 5);
}

void USootSpriteLimb::TickUpdate(float DeltaTime)
{
	PROFILE_FUNCTION()
	
	if (MeshInstance == -1)
	{
		LIMB_ISM()->RequestInstance(this);
		ResetJointPoses();
	}
	
	// grow to the target length
	CreatureMath::LinearStep(Length, TargetLength, 1200 * DeltaTime);
	
	// todo: batchupdateInstancetransforms
	
	if (MeshInstance != -1)
	{
		Bend();
		DetermineJointRotations();
		PackSkinningMatrices();
		LIMB_ISM()->UpdateInstance(this);
	}
}

void USootSpriteLimb::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Clear();
	Super::EndPlay(EndPlayReason);
}

void USootSpriteLimb::Clear()
{
	USootSpriteLimbISM* LimbIsm = LIMB_ISM();
	if (MeshInstance != -1)
	{
		LimbIsm->ReturnInstance(this);
	}
	AttachTarget = {};
}

void USootSpriteLimb::SetGrowState(ESootSpriteLimbGrowState InGrowState)
{
	GrowState = InGrowState;
}

void USootSpriteLimb::BeginPlay()
{
	Super::BeginPlay();
	PackedSkinningMatrices.SetNum(JOINT_PARAMETER_COUNT);
	ResetJointPoses();
}

void USootSpriteLimb::SetTargetDirection(const FVector& Direction, double NormalOffset)
{
	check(Direction.IsNormalized())
	// demo limb-itation (heh)
	check(AttachTarget.Type == ELimbAttachTargetType::World)
	const FVector TrBegin = GetOrigin();
	const FVector TrEnd = TrBegin + Direction * Length;
	CreatureMath::Raycast(GetWorld(), AttachTarget.Offset, TrBegin, TrEnd, GameTraceChannel::Ground, NormalOffset);
}

FSootSpriteLimbResult USootSpriteLimb::InitializeLegTargetInterpolation(const FVector2D& AnchorOffset, double MaxStepHeight, double EarlyOutStepHeight, bool bDebugDraw)
{
	check(AttachTarget.Type == ELimbAttachTargetType::World)
	
	constexpr float DEBUG_DRAW_TIME = 3.0f;
	
	const double FinalDistance = AnchorOffset.Size();
	if (FMath::IsNaN(FinalDistance) || FinalDistance == INFINITY)
	{
		return {FSootSpriteLimbResult::InvalidInput};
	}
	
	FSootSpriteLimbResult Result = {FSootSpriteLimbResult::None};
	TargetInterpBegin = AttachTarget.Offset;
	
	const FVector FinalOffset3D(AnchorOffset.X, AnchorOffset.Y, 0);
	
	const double TR_HEIGHT = MaxStepHeight * 10;
	
	constexpr int32 MAX_STEPS = 5;
	constexpr double REFERENCE_DISTANCE_FOR_MAX_STEPS = 250.0;
	constexpr double REFERENCE_STEP_DISTANCE = REFERENCE_DISTANCE_FOR_MAX_STEPS / MAX_STEPS;
	
	const int32 StepCount = FMath::Min(FMath::DivideAndRoundUp(FinalDistance, REFERENCE_STEP_DISTANCE), 5);
	const double StepFactor = 1.0 / StepCount;
	
	FVector SubstepOrigin = GetOrigin();
	FHitResult HitResult;
	
	FColor DebugMissLineColor = FColor::Red;
	FColor DebugHitLineColor = FColor::Green;
	
	// substep forward from the current position, looking out for walls/cliffs, 
	// and early exiting when a short step is found. this is done so that the 
	// foot will be placed on the ledge of the step rather than far from the 
	// ledge, which makes the walking creature look like they're triple-stepping
	// a stairmaster at the gym.
	for (int32 Step = 1; Step <= StepCount; Step++)
	{
		const double Progress = (double) Step * StepFactor;
		const FVector Offset3D = FinalOffset3D * Progress;
		const FVector TrBegin = (SubstepOrigin + Offset3D) + FVector::UpVector * TR_HEIGHT;
		const FVector TrEnd = TrBegin + FVector::DownVector * (TR_HEIGHT * 2);
		
		FVector NewTargetInterpEnd(0);
		if (!CreatureMath::Raycast(GetWorld(), NewTargetInterpEnd, TrBegin, TrEnd, GameTraceChannel::Ground, 0, &HitResult))
		{
			Result.Message = FSootSpriteLimbResult::RaycastDidNotHit;
			break;
		}
		
		const double ZDiff = NewTargetInterpEnd.Z - TargetInterpEnd.Z;
		if (FMath::Abs(ZDiff) > MaxStepHeight)
		{
			DebugHitLineColor = FColor::Red;
			Result.Message = FSootSpriteLimbResult::RaycastHitOverMaxStepHeight;
			break;
		}
		
		TargetInterpEnd = NewTargetInterpEnd;
		Result.InitializeLegTargetInterpolation.Progress = Progress;
		
		if (FMath::Abs(ZDiff) > EarlyOutStepHeight)
		{
			Result.Message = FSootSpriteLimbResult::RaycastHitOverMaxEarlyOutStepHeight;
			break;
		}
		
		SubstepOrigin.Z += ZDiff;
	}
	
	if (bDebugDraw)
	{
		CreatureDebugDraw::HitResult(GetWorld(), HitResult, DEBUG_DRAW_TIME, DebugMissLineColor, DebugHitLineColor);
	}
	
	return Result;
}

void USootSpriteLimb::InterpolateTarget(float Alpha)
{
	const double ZAlpha = CreatureMath::SampleNormalCDF(Alpha);
	const FVector NormalSmoothed = TargetInterpEnd * ZAlpha + TargetInterpBegin * (1-ZAlpha);
	const FVector Linear = TargetInterpEnd * Alpha + TargetInterpBegin * (1-Alpha);
	const FVector Smoothed = FMath::Lerp(Linear, NormalSmoothed, 0.35);
	
	const FVector HeightOffset = FVector::UpVector * ASootSprite::WalkLimbHeightSampler.LinearSample(Alpha);
	
	AttachTarget.Offset = Smoothed + HeightOffset;
}

FVector USootSpriteLimb::GetBindJointPosition(int32 i)
{
	check(i >= 0 && i < JOINT_COUNT)
	const double Alpha = (double)i / (double)(JOINT_COUNT - 1);
	return FVector(0, 0, MESH_LENGTH * Alpha);
}

FVector USootSpriteLimb::GetOrigin() const
{
	if (AttachOrigin.Type == ELimbAttachTargetType::Actor)
	{
		return GetSootSprite()->GetActorLocation() + GetSootSprite()->GetActorRotation().RotateVector(AttachOrigin.Offset);
	}
	else
	{
		return AttachOrigin.Offset;
	}
}

const FVector& USootSpriteLimb::GetTarget() const
{
	check(AttachTarget.Type == ELimbAttachTargetType::World)
	return AttachTarget.Offset;
}

ASootSprite* USootSpriteLimb::GetSootSprite() const
{
	return Cast<ASootSprite>(AttachOrigin.Actor);
}

void USootSpriteLimb::ResetJointPoses()
{
	for (int32 i = 0; i < JOINT_COUNT; i++)
	{
		JointPoses[i] = FTransform(
			FQuat::Identity, 
			GetBindJointPosition(i), 
			FVector::OneVector
		);
	}
	PackSkinningMatrices();
}

void USootSpriteLimb::DetermineJointRotations()
{
	PROFILE_FUNCTION()
	
	for (int32 i = 0; i < JOINT_COUNT; i++)
	{
		const int32 MinIndex = FMath::Max(i-1, 0);
		const int32 MaxIndex = FMath::Min(i+1, JOINT_COUNT-1);
		FVector PointInDirection = JointPoses[MaxIndex].GetLocation() - JointPoses[MinIndex].GetLocation();
		
		if (!PointInDirection.Normalize())
		{
			PointInDirection = FVector::UpVector;
		}
		
		const FVector Location = JointPoses[i].GetLocation();		
		const FQuat Rotation = FQuat::FindBetweenNormals(FVector::UpVector, PointInDirection);
		JointPoses[i].SetComponents(Rotation, Location, FVector::OneVector);
	}
}

void USootSpriteLimb::PackSkinningMatrices()
{
	PROFILE_FUNCTION()
	
	PackedSkinningMatrices.SetNum(JOINT_PARAMETER_COUNT);
	
	for (int32 i = 0; i < JOINT_COUNT; i++)
	{
		const FTransform& Pose = JointPoses[i];
		const FQuat Rotation = Pose.GetRotation().GetNormalized();
		const FVector Translation = Pose.GetLocation() - Rotation.RotateVector(GetBindJointPosition(i));
		
		const FVector BasisX = Rotation.RotateVector(FVector::XAxisVector);
		const FVector BasisY = Rotation.RotateVector(FVector::YAxisVector);
		const FVector BasisZ = Rotation.RotateVector(FVector::ZAxisVector);
		
		const int32 Base = i * PARAMETERS_PER_JOINT;
		
		// row 1
		PackedSkinningMatrices[Base + 0] = BasisX.X;
		PackedSkinningMatrices[Base + 1] = BasisY.X;
		PackedSkinningMatrices[Base + 2] = BasisZ.X;
		PackedSkinningMatrices[Base + 3] = Translation.X;
		// row 2
		PackedSkinningMatrices[Base + 4] = BasisX.Y;
		PackedSkinningMatrices[Base + 5] = BasisY.Y;
		PackedSkinningMatrices[Base + 6] = BasisZ.Y;
		PackedSkinningMatrices[Base + 7] = Translation.Y;
		// row 3
		PackedSkinningMatrices[Base + 8] = BasisX.Z;
		PackedSkinningMatrices[Base + 9] = BasisY.Z;
		PackedSkinningMatrices[Base + 10] = BasisZ.Z;
		PackedSkinningMatrices[Base + 11] = Translation.Z;
	}
}

// todo: batch
void USootSpriteLimb::SetJointPosition(int32 Joint, const FVector& WorldPosition)
{
	check(Joint > 0)
	const FVector OriginOffset = WorldPosition - GetOrigin();
	JointPoses[Joint].SetLocation(OriginOffset);
}

void USootSpriteLimb::Bend()
{
	PROFILE_FUNCTION()
	
	const FVector Origin = GetOrigin();
	const FVector Target = GetTarget();
	const FVector TargetDiff = Target - Origin;
	const double Dist = TargetDiff.Size();
	const FVector TargetDir = TargetDiff / Dist;
	
	float CurveScale;
	if (bAllowBend
		&& Dist < Length 
		&& CreatureMath::SolveCatenary(Dist * 0.5, Length, CurveScale))
	{
		constexpr double BLEND_SPEED = 8.0;
		
		// default bend is the limb's endpoint direction + forward and a little up
		const FVector ActorForward = GetSootSprite()->GetActorForwardVector();
		FVector BaseTargetBendDir = (TargetDiff + FVector::UpVector * 200.0f + ActorForward * 100).GetSafeNormal();
		
		// try to bend around obstacles
		
		int32 FindBendDirStepCount = 0;
		FVector RotationAxis, Unused;
		CreatureMath::MakeBasis(BaseTargetBendDir, RotationAxis, Unused);
		if ((TargetDir | ActorForward) < 0)
		{
			RotationAxis *= -1;
		}
		const FVector EnvironmentAwareBendDir = FindBendDirection(FindBendDirStepCount, Origin,  BaseTargetBendDir, RotationAxis, 30);
	
		// if the way isn't clear, bend away from hazards. else, just use default bend
		if (FindBendDirStepCount > 0)
		{
			BaseTargetBendDir = EnvironmentAwareBendDir;
		}
		
		// cowboy (wide knees). little hacky. would be better to have a controllable knee/bend direction.
		const FVector Cowboy = CowboyDir * (GetSootSprite()->Settings.Cowboy /*+ BiasedNormCloseToGround*/);
		
		const FVector TargetBendDir = (BaseTargetBendDir + Cowboy).GetSafeNormal();
		CreatureMath::LinearStep(BendDir, TargetBendDir, BLEND_SPEED * GetWorld()->DeltaTimeSeconds, true);
		
		const FVector MidPoint = (Origin + Target) * 0.5;
		
		// place joints along the curve
		
		const float H = CreatureMath::SampleCatenary(CurveScale, 0.5f * Length).Y;
		for (int32 i = 1; i < 4; i++)
		{
			const float S = Length * ((float)i / 4 - 0.5f);
			const FVector2f P = CreatureMath::SampleCatenary(CurveScale, S);
			SetJointPosition(i, MidPoint + TargetDir * P.X + BendDir * (H - P.Y));
		}
		SetJointPosition(4, Target);
	}
	else
	{
		// point limb straight in the target direction
		// todo: should interpolate into straightness, as is done above with the curve
		SetJointPosition(1, Origin + TargetDir * (Length / 4));
		SetJointPosition(2, Origin + TargetDir * (2 * Length / 4));
		SetJointPosition(3, Origin + TargetDir * (3 * Length / 4));
		SetJointPosition(4, Origin + TargetDir * Length);
	}
}

FVector USootSpriteLimb::FindBendDirection(int32& OutStepCount, const FVector& Origin, const FVector& Dir, const FVector& RotationAxis, double Dist, double ClearanceAngle)
{
	OutStepCount = 0;
	FVector OutBendDir = Dir;
	
	const FVector TrBegin = Origin;
	FVector HitLocation;	
	FVector TrEnd = Origin + OutBendDir * (Dist * 0.8);
	
	constexpr int32 STEP_COUNT = 5;
	constexpr double STEP_SIZE = 15;
	
	while (OutStepCount < STEP_COUNT && CreatureMath::Raycast(GetWorld(), HitLocation, TrBegin, TrEnd, GameTraceChannel::Ground))
	{
		OutBendDir = OutBendDir.RotateAngleAxis(STEP_SIZE, RotationAxis);
		TrEnd = Origin + OutBendDir * (Dist * 0.8);
		OutStepCount++;
	} 
	
	return OutBendDir.RotateAngleAxis(ClearanceAngle, RotationAxis);
}
