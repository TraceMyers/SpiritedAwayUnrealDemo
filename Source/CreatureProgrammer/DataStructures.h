#pragma once

#include "CoreMinimal.h"

enum class ERWMode { Read, Write };

// a spin lock that either allows lots of readers, or one writer.
struct FReadWriteLock
{
	FReadWriteLock(FThreadSafeCounter* InLock, ERWMode InMode) : Mode(InMode), Lock(InLock)
	{
		if (Mode == ERWMode::Read)
		{
			while (true)
			{
				const int32 Key = Lock->Increment();
				// checking for read usage without returning keys
				check(Key <= MAX_READERS);
				// if 0, would indicate that too many readers have slammed the lock while one writer is writing
				check(Key != 0);
				if (Key > 0)
				{
					break;
				}
				Lock->Decrement();
			}
		}
		else
		{
			while (true)
			{
				const int32 Key = Lock->Subtract(MAX_READERS + 1);
				if (Key == 0)
				{
					break;
				}
				Lock->Add(MAX_READERS + 1);
			}
		}
	}

	~FReadWriteLock()
	{
		if (Mode == ERWMode::Read)
		{
			Lock->Decrement();
		}
		else
		{
			Lock->Add(INT16_MAX);
		}
	}

protected:
	static constexpr int32 MAX_READERS = INT16_MAX;
	ERWMode Mode = ERWMode::Write;
	FThreadSafeCounter* Lock = nullptr;
};

struct FBitArray
{
	struct FIndexer
	{
		FIndexer(int32 i)
		{
			Slot = i / 64;		
			Bit = i - Slot * 64;
		}
		int32 Slot = 0;
		int32 Bit = 0;
	};
	
	void SetCapacity(int32 Cap)
	{
		if (Cap <= 0)
		{
			check(Cap == 0)
			Slots.Empty();
		}
		else
		{
			FIndexer Ind(Cap-1);
			Slots.SetNum(Ind.Slot + 1);
		}
		Capacity = Cap;
	}
	
	FORCEINLINE int32 GetCapacity() const { return Capacity; }
	
	void SetBit(int32 i)
	{
		check(i < Capacity)
		FIndexer Ind(i);
		Slots[Ind.Slot] |= 1ULL << Ind.Bit;
	}
	
	void ClearBit(int32 i, bool bCheckSet=false)
	{
		check(i < Capacity)
		FIndexer Ind(i);
		if (bCheckSet)
		{
			check((Slots[Ind.Slot] & (1ULL << Ind.Bit)) != 0);
		}
		Slots[Ind.Slot] &= ~(1ULL << Ind.Bit);
	}
	
	bool IsBitSet(int32 i) const
	{
		check(i < Capacity)
		FIndexer Ind(i);
		return (Slots[Ind.Slot] & (1ULL << Ind.Bit)) != 0;;	
	}
	
	FORCEINLINE bool operator[](int32 i) const
	{
		return IsBitSet(i);
	}
	
	int32 FindFirstClearedBit(int32 StartIndex=0) const
	{
		FIndexer Ind(StartIndex);
		int32 StartBit = Ind.Bit;
		
		for (int32 i = Ind.Slot; i < Slots.Num(); i++)
		{
			if (Slots[i] != UINT64_MAX)
			{
				int32 BitCap = 64;
				if (i == Slots.Num() - 1)
				{
					FIndexer CapInd(Capacity-1);
					BitCap = CapInd.Bit + 1;
				}
				// I recall somebody saying that using userland storage like this might optimize faster
				// than using a 1ULL literal in the loop. haven't tested it.
				uint64 Bit = 1;
				for (int32 j = StartBit; j < BitCap; j++)
				{
					if ((Slots[i] & Bit) == 0)
					{
						return j;
					}
					Bit <<= 1;
				}
				StartBit = 0;
			}
		}
		return -1;
	}
	
protected:
	
	TArray<uint64> Slots;
	int32 Capacity = 0;
};

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
		FReadWriteLock Lock(&Counter, ERWMode::Write);
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
		FReadWriteLock Lock(&Counter, ERWMode::Write);
		Availability.ClearBit(Slot.Index, true);
		MinAvailable = FMath::Min(MinAvailable, Slot.Index);
		Slot.Index = -1;
	}
	
	bool IsSlotAvailable(int32 Index)
	{
		FReadWriteLock Lock(&Counter, ERWMode::Read);
		return Availability[Index];
	}

protected:
	
	FBitArray Availability;
	int32 MinAvailable = 0;
	FThreadSafeCounter Counter;
};
