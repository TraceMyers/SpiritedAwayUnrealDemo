#pragma once

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
