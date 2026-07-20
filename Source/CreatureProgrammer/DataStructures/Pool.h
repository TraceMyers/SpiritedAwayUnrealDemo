#pragma once

#include "BitArray.h"
#include "CreatureProgrammer/Multithreading/ThreadingTypes.h"

// toy type for FPoolFrontend
struct FPoolSlot
{
	int32 Index = -1;
};

// the front end / bookkeeper for a pool. useful in cases where the backend is an array
// that you freely control, but needs to be stored elsewhere. this is just the books part.
struct FPoolFrontend
{
	void Request(FPoolSlot& Slot)
	{
		check(Slot.Index == -1)
		FScopeReadWriteLock Lock(&Counter, ERWMode::Write);
		if (MinAvailable == -1 || MinAvailable >= Availability.GetCapacity())
		{
			MinAvailable = Availability.GetCapacity();
			Availability.SetCapacity(FMath::Max(Availability.GetCapacity() * 2, 1024));
		}
		Slot.Index = MinAvailable;
		Availability.SetBit(Slot.Index);
		MinAvailable = Availability.FindFirstClearedBit(MinAvailable+1);
	}
	
	void Return(FPoolSlot& Slot)
	{
		FScopeReadWriteLock Lock(&Counter, ERWMode::Write);
		Availability.ClearBit(Slot.Index, true);
		MinAvailable = FMath::Min(MinAvailable, Slot.Index);
		Slot.Index = -1;
	}
	
	bool IsSlotAvailable(int32 Index)
	{
		FScopeReadWriteLock Lock(&Counter, ERWMode::Read);
		return Availability[Index];
	}

protected:
	
	FBitArray Availability;
	int32 MinAvailable = 0;
	FThreadSafeCounter Counter;
};