#pragma once

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