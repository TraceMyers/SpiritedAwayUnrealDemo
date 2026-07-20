#pragma once

// a simple spin lock for simple needs. lock an execution scope to one thread at a time.
struct FScopeSpinLock
{
	FScopeSpinLock(FThreadSafeCounter* InLock, bool bLockCondition=true) : Lock(InLock)
	{
		if (!bLockCondition)
		{
			return;
		}
		while (true)
		{
			const int32 Key = Lock->Increment();
			if (Key == 1) // was 0 before increment
			{
				bLocked = true;
				return;
			}
			Lock->Decrement();
			FPlatformProcess::Sleep(0); // thread context switch
		}
	}
	
	~FScopeSpinLock()
	{
		if (!bLocked)
		{
			return;
		}
		check(Lock->GetValue() > 0) // might catch a problem, but ofc not atomic with decrement
		Lock->Decrement();
	}
	
protected:
	FThreadSafeCounter* Lock = nullptr;
	bool bLocked = false;
};

enum class ERWMode { Read, Write };

// a spin lock that either allows lots of readers, or one writer at a time within an execution scope.
struct FScopeReadWriteLock
{
	FScopeReadWriteLock(FThreadSafeCounter* InLock, ERWMode InMode) : Mode(InMode), Lock(InLock)
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
					return;
				}
				Lock->Decrement();
				FPlatformProcess::Sleep(0);// thread context switch
			}
		}
		else
		{
			constexpr int32 KEY_STEP_SIZE = MAX_READERS + 1;
			while (true)
			{
				const int32 Key = Lock->Subtract(KEY_STEP_SIZE);
				if (Key == 0)
				{
					return;
				}
				Lock->Add(KEY_STEP_SIZE);
				FPlatformProcess::Sleep(0);// thread context switch
			}
		}
	}

	~FScopeReadWriteLock()
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

template<typename T>
struct FLoan
{
	FLoan(T* InData, FThreadSafeCounter* InCounter) : Data(InData), Counter(InCounter)
	{
		Counter->Increment();
	}
		
	~FLoan()
	{
		Counter->Decrement();
	}
		
	FLoan& operator=(const FLoan& Other)
	{
		Data = Other.Data;
		return *this;
	}
		
public:
	T* Data = nullptr; 
protected:
	mutable FThreadSafeCounter* Counter = nullptr;
};

template<typename T>
struct FConstLoan
{
	FConstLoan(const T* InData, FThreadSafeCounter* InCounter) : Data(InData), Counter(InCounter)
	{
		Counter->Increment();
	}
		
	~FConstLoan()
	{
		Counter->Decrement();
	}
		
	FConstLoan& operator=(const FConstLoan& Other)
	{
		Data = Other.Data;
		return *this;
	}
		
public:
	const T* Data = nullptr; 
protected:
	mutable FThreadSafeCounter* Counter = nullptr;
};

// for reference counting loaned out data.
template <typename T>
struct FLender
{
	FLoan<T> TakeLoan()
	{
		return FLoan<T>(&Data, &Counter);
	}
	
	FConstLoan<T> TakeConstLoan() const
	{
		return FConstLoan<T>(&Data, &Counter);
	}
	
	FLender& operator=(const FLender& Other)
	{
		check(Counter.GetValue() == 0)
		Data = Other.Data;
		return *this;
	}
	
	int32 GetCounterValue() const
	{
		return Counter.GetValue();
	}
	
protected:
	T Data;
	mutable FThreadSafeCounter Counter;
};