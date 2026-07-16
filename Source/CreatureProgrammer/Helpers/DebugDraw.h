#pragma once

namespace CreatureDebugDraw
{
	void HitResult(const UWorld* World, const FHitResult& HitResult, float Time=-1, FColor MissLineColor=FColor::Red, FColor HitLineColor=FColor::Green);
}