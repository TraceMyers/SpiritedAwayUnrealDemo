#include "CreatureVisionUpdate.h"

#include "CreatureProgrammer/Creatures/Creature.h"
#include "CreatureProgrammer/GameBase/GooberGameState.h"
#include "CreatureProgrammer/Helpers/MacroHelpers.h"

void FCreatureVisionUpdate::Prepare(const UWorld* World, int32 First, int32 Count)
{
	PROFILE_FUNCTION()
	
	const FCreatureDatabase& Database = WORLD_GAME_STATE(World)->GetCreatureDatabase();
	FConstLoan<FCreatureBulkAIData> AILoaner = Database.AI.TakeConstLoan();
	const FCreatureBulkAIData& AI = *AILoaner.Data; 
	
	ForwardVectors = AI.CreatureForwardVectors.Slice(First, Count);
	VisionOrigins = AI.CreatureVisionConeLocations.Slice(First, Count);
	CosVisionAngles = TConstArrayView<double>(AI.CreatureVisionCosHalfAngles).Slice(First, Count);
	VisionDistances = TConstArrayView<double>(AI.CreatureVisionMaxDistances).Slice(First, Count);
}

// todo: make monolithic data read only without sucking
void FCreatureVisionUpdate::Execute(const UWorld* World, int32 First, int32 Count)
{
	PROFILE_FUNCTION()
	
	const FCreatureDatabase& Database = WORLD_GAME_STATE(World)->GetCreatureDatabase();
	
	const FConstLoan<FCreatureBulkAIData> AILoaner = Database.AI.TakeConstLoan();
	const FCreatureBulkAIData& AI = *AILoaner.Data; 
	
	const int32 CreatureCount = Database.CreaturesNum();
	
	check(First % 4 == 0)
	check(First + Count <= CreatureCount)
	
	// all creature datas
	const FSimdVector3Array& CreatureLocations = AI.CreatureLocations;
	const TArray<double>& CreatureRadii = AI.CreatureVisibleSphereRadii;
	
	const int32 CreatureTargetChunkCount = CreatureLocations.Chunks.Num();
	
	// todo: this can be a procedure. can use for creatures and inanimates.
	for (int32 i = 0; i < Count; i++)
	{
		const TPair<int32, int32> vi = Simd::VectorIndex(i);
		
		SimdVector VisionOriginX = Simd::Splat(VisionOrigins[vi.Key].X[vi.Value]);
		SimdVector VisionOriginY = Simd::Splat(VisionOrigins[vi.Key].Y[vi.Value]);
		SimdVector VisionOriginZ = Simd::Splat(VisionOrigins[vi.Key].Z[vi.Value]);
		
		SimdVector VisionDirX = Simd::Splat(ForwardVectors[vi.Key].X[vi.Value]);
		SimdVector VisionDirY = Simd::Splat(ForwardVectors[vi.Key].Y[vi.Value]);
		SimdVector VisionDirZ = Simd::Splat(ForwardVectors[vi.Key].Z[vi.Value]);
		
		SimdVector CosVisionAngle = Simd::Splat(CosVisionAngles[i]);
		SimdVector VisionDistance = Simd::Splat(VisionDistances[i]);
		
		for (int32 j = 0; j < CreatureTargetChunkCount; j++)
		{
			SimdVector CreaturePosX = CreatureLocations.LoadX(j);
			SimdVector CreaturePosY = CreatureLocations.LoadY(j);
			SimdVector CreaturePosZ = CreatureLocations.LoadZ(j);
			
			SimdVector DiffX = Simd::Sub(CreaturePosX, VisionOriginX);
			SimdVector DiffY = Simd::Sub(CreaturePosY, VisionOriginY);
			SimdVector DiffZ = Simd::Sub(CreaturePosZ, VisionOriginZ);
			
			SimdVector DistAlongFore = Simd::Dot3D(DiffX, DiffY, DiffZ, VisionDirX, VisionDirY, VisionDirZ);
			SimdVector Dist = Simd::Size3D(DiffX, DiffY, DiffZ);
			SimdVector CosTheta = Simd::Div(DistAlongFore, Dist);
			
			SimdVector WithinVisionAngle = Simd::GreaterThanOrEqual(CosTheta, CosVisionAngle);
			
			SimdVector Radii = Simd::Load(&CreatureRadii[j * 4]);
			SimdVector SpherePointDist = Simd::Sub(DistAlongFore, Radii);
			SimdVector WithinVisionDist = Simd::LessThanOrEqual(SpherePointDist, VisionDistance);
			
			SimdVector WithinVision = Simd::And(WithinVisionAngle, WithinVisionDist);
			
			double CreatureIsWithinVision[4];
			Simd::Store(WithinVision, &CreatureIsWithinVision[0]);
			
			for (int32 k = 0; k < 4; k++)
			{
				const uint64 Visibility = *reinterpret_cast<uint64*>(&CreatureIsWithinVision[k]);
				const bool bVisible = Visibility == UINT64_MAX;
				const int32 Beholder = First + i;
				const int32 Beheld = j * 4 + k;
				if (bVisible && Beheld < CreatureCount)
				{
					WhoSeesWho.Emplace(TPair<int16, int16>(Beholder, Beheld));
				}
			}
		}
	}
}

void FCreatureVisionUpdate::ApplyResults(const UWorld* World, int32 First, int32 Count)
{
	PROFILE_FUNCTION()
	
	check(IsInGameThread())
	
	const FCreatureDatabase& Database = WORLD_GAME_STATE(World)->GetCreatureDatabase();
	const TArray<ACreature*>& Creatures = Database.GetCreatures();
	
	if (Count == 0)
	{
		return;
	}
	
	const int32 Last = First + Count - 1;
	
	TArray<ACreature*> NewVisibleCreatures;
	NewVisibleCreatures.Reserve(1024);
	
	for (int i = 0; i < WhoSeesWho.Num(); i++)
	{
		TPair VisionPair = WhoSeesWho[i];
		const int16 BeholderIndex = VisionPair.Key;
		const int16 BeheldIndex = VisionPair.Value;
		
		check(BeholderIndex >= First && BeholderIndex <= Last)
		
		ACreature* Beholder = Creatures[BeholderIndex];
		ACreature* Beheld = Creatures[BeheldIndex];
		
		const int32 CurIndex = Beholder->VisibleCreatures.Find(Beheld);
		if (CurIndex == -1)
		{
			Beholder->OnCreatureEnterVision(Beheld);
		}
		else
		{
			Beholder->VisibleCreatures.RemoveAtSwap(CurIndex, 1, EAllowShrinking::No);
		}
		
		NewVisibleCreatures.Emplace(Beheld);
		
		if (i == WhoSeesWho.Num() - 1 || BeholderIndex != WhoSeesWho[i+1].Key)
		{
			for (ACreature* Creature : Beholder->VisibleCreatures)
			{
				Beholder->OnCreatureExitVision(Creature);
			}
			Beholder->VisibleCreatures = NewVisibleCreatures;
			NewVisibleCreatures.Empty(NewVisibleCreatures.Max());
		}
	}
}
