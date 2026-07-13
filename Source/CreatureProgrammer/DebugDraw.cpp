#include "DebugDraw.h"

void CreatureDebugDraw::HitResult(const UWorld* World, const FHitResult& HitResult, float Time, FColor MissLineColor, FColor HitLineColor)
{
	DrawDebugPoint(World, HitResult.TraceStart, 5, FColor::Yellow, false, Time);
	if (HitResult.bBlockingHit)
	{
		DrawDebugLine(World, HitResult.TraceStart, HitResult.ImpactPoint, HitLineColor, false, Time, 0, 3);
		if (HitResult.GetActor())
		{
			const FBox HitActorBox = HitResult.GetActor()->GetComponentsBoundingBox();
			DrawDebugBox(World, HitActorBox.GetCenter(), HitActorBox.GetExtent(), FColor::Magenta, false, Time, 0, 4);
		}
		FVector XAxis(0,0,1), YAxis(0,1,0);
		if ((HitResult.ImpactNormal | FVector::ZAxisVector) > 0.99)
		{
			XAxis = HitResult.ImpactNormal.Cross(FVector::XAxisVector).GetSafeNormal();
			YAxis = HitResult.ImpactNormal.Cross(XAxis).GetSafeNormal();
		}
		DrawDebugCircle(World, HitResult.ImpactPoint, 50, 16, FColor::Emerald, false, Time, 0, 2, XAxis, YAxis);
	}
	else
	{
		DrawDebugLine(World, HitResult.TraceStart, HitResult.TraceEnd, MissLineColor, false, Time, 0, 3);
	}
}
